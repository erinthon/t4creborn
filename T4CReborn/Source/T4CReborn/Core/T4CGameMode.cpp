#include "Core/T4CGameMode.h"
#include "Core/T4CGameState.h"
#include "Core/T4CPlayerState.h"
#include "Player/T4CCharacter.h"
#include "Player/T4CPlayerController.h"
#include "AI/T4CMonster.h"
#include "Items/T4CLootPickup.h"
#include "Items/T4CItemData.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CAbilityInputID.h"
#include "GAS/T4CGameplayTags.h"
#include "Attributes/T4CClassData.h"
#include "Items/T4CInventoryComponent.h"
#include "Items/T4CItemData.h"
#include "AbilitySystemComponent.h"
#include "UI/T4CHUD.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AT4CGameMode::AT4CGameMode()
{
	DefaultPawnClass = AT4CCharacter::StaticClass();
	PlayerControllerClass = AT4CPlayerController::StaticClass();
	PlayerStateClass = AT4CPlayerState::StaticClass();
	GameStateClass = AT4CGameState::StaticClass();
	HUDClass = AT4CHUD::StaticClass();

	MonsterClass = AT4CMonster::StaticClass();
	LootPickupClass = AT4CLootPickup::StaticClass();

	// Pontos de spawn afastados dos PlayerStarts (jogadores em y=+-300),
	// para o jogador se aproximar e engajar 1 de cada vez, sem ser cercado no spawn.
	MonsterSpawnPoints = {
		FVector(1900.f, 0.f, 120.f),
		FVector(-1900.f, 700.f, 120.f),
		FVector(900.f, -2000.f, 120.f),
		FVector(-1200.f, -1600.f, 120.f)
	};
}

void AT4CGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (const FVector& Point : MonsterSpawnPoints)
	{
		SpawnMonster(Point);
	}

	// Autosave periódico dos personagens (persistência).
	GetWorldTimerManager().SetTimer(AutoSaveTimer, this, &AT4CGameMode::SaveAllPlayers,
		AutoSaveInterval, true, AutoSaveInterval);

	bAutoTest = FParse::Param(FCommandLine::Get(), TEXT("T4CAutoTest"));
	if (bAutoTest)
	{
		UE_LOG(LogTemp, Display, TEXT("[T4C][AutoTest] ativado"));
		GetWorldTimerManager().SetTimer(AutoTestTimer, this, &AT4CGameMode::RunAutoTest, 2.0f, true, 3.0f);
	}
}

void AT4CGameMode::Logout(AController* Exiting)
{
	if (Exiting)
	{
		if (AT4CPlayerState* PS = Exiting->GetPlayerState<AT4CPlayerState>())
		{
			PS->SaveCharacter();
		}
	}
	Super::Logout(Exiting);
}

void AT4CGameMode::SaveAllPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (AT4CPlayerState* PS = PC->GetPlayerState<AT4CPlayerState>())
			{
				PS->SaveCharacter();
			}
		}
	}
}

void AT4CGameMode::RunAutoTest()
{
	++AutoTestTick;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		AT4CPlayerState* PS = PC ? PC->GetPlayerState<AT4CPlayerState>() : nullptr;
		if (!PS)
		{
			continue;
		}

		// 1) Garante a classe Guerreiro (Q=Powerful Blow projétil, E=Parry).
		if (!PS->HasChosenClass())
		{
			PS->ServerSelectClass(ET4CClass::Warrior);
			UE_LOG(LogTemp, Display, TEXT("[T4C][AutoTest] %s -> classe Guerreiro"), *PS->GetPlayerName());
			continue;
		}

		// 2) Personagem novo (inventário vazio): dá arma+armadura, ganha XP e SALVA
		//    imediatamente — assim o slot existe para o próximo lançamento carregar.
		if (UT4CInventoryComponent* Inv = PS->GetInventory())
		{
			if (Inv->GetItems().Num() == 0)
			{
				for (const FT4CItem& Item : T4CItems::DropTable())
				{
					if (Item.Type == ET4CItemType::Weapon && Item.WeaponDamage >= 10.f) { Inv->AddItem(Item); break; }
				}
				for (const FT4CItem& Item : T4CItems::DropTable())
				{
					if (Item.Type == ET4CItemType::Armor && Item.Armor >= 7.f) { Inv->AddItem(Item); break; }
				}
				PS->GrantExperience(250); // sobe de nível de forma determinística
				PS->SaveCharacter();
			}
		}

		// 3) Usa Q (2x: a 2ª cai no cooldown) e E (Parry) via o caminho do input.
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		const UT4CAttributeSet* Set = ASC ? ASC->GetSet<UT4CAttributeSet>() : nullptr;
		if (ASC && Set)
		{
			ASC->AbilityLocalInputPressed(static_cast<int32>(ET4CAbilityInputID::AbilityQ));
			ASC->AbilityLocalInputPressed(static_cast<int32>(ET4CAbilityInputID::AbilityQ));
			ASC->AbilityLocalInputPressed(static_cast<int32>(ET4CAbilityInputID::AbilityE));
			UE_LOG(LogTemp, Display, TEXT("[T4C][AutoTest] %s | Arma+%.0f Armadura+%.0f Reducao%.0f%% (HP %.0f/%.0f)"),
				*PS->GetPlayerName(), Set->GetWeaponDamageBonus(), Set->GetArmor(),
				Set->GetDamageReduction() * 100.f, Set->GetHealth(), Set->GetMaxHealth());
		}
	}
}

AActor* AT4CGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> Starts;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		Starts.Add(*It);
	}

	if (Starts.Num() == 0)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	// Round-robin: garante pontos distintos para jogadores consecutivos.
	APlayerStart* Chosen = Starts[NextStartIndex % Starts.Num()];
	NextStartIndex++;
	return Chosen;
}

void AT4CGameMode::SpawnMonster(FVector Location)
{
	if (!MonsterClass)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AT4CMonster* Spawned = GetWorld()->SpawnActor<AT4CMonster>(MonsterClass, Location, FRotator::ZeroRotator, Params);
	UE_LOG(LogTemp, Display, TEXT("[T4C] Monstro spawnado em %s (%s)"),
		*Location.ToString(), Spawned ? TEXT("ok") : TEXT("FALHOU"));
}

void AT4CGameMode::RespawnPlayer(AController* Controller)
{
	if (!Controller)
	{
		return;
	}
	RestartPlayer(Controller);
	UE_LOG(LogTemp, Display, TEXT("[T4C] Jogador respawnado: %s"), *Controller->GetName());
}

void AT4CGameMode::OnMonsterKilled(const FVector& SpawnLocation)
{
	// Respawn após um atraso, mantendo a população de monstros.
	FTimerHandle Handle;
	FTimerDelegate Del = FTimerDelegate::CreateUObject(this, &AT4CGameMode::SpawnMonster, SpawnLocation);
	GetWorldTimerManager().SetTimer(Handle, Del, MonsterRespawnDelay, false);
}

void AT4CGameMode::DropLoot(const FVector& DeathLocation)
{
	if (!LootPickupClass || FMath::FRand() > LootDropChance)
	{
		return; // sem drop desta vez
	}

	const FT4CItem Item = T4CItems::Roll();
	if (!Item.IsValid())
	{
		return;
	}

	// Pousa o saco logo acima do chão, no local da morte.
	const FVector Loc = DeathLocation + FVector(0.f, 0.f, 20.f);
	const FTransform SpawnTM(FRotator::ZeroRotator, Loc);

	if (AT4CLootPickup* Pickup = GetWorld()->SpawnActorDeferred<AT4CLootPickup>(
		LootPickupClass, SpawnTM, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
	{
		Pickup->SetItem(Item);
		Pickup->FinishSpawning(SpawnTM);
		UE_LOG(LogTemp, Display, TEXT("[T4C] Loot dropado: %s em %s"), *Item.Name, *Loc.ToString());
	}
}
