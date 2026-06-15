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

	bAutoTest = FParse::Param(FCommandLine::Get(), TEXT("T4CAutoTest"));
	if (bAutoTest)
	{
		UE_LOG(LogTemp, Display, TEXT("[T4C][AutoTest] ativado"));
		GetWorldTimerManager().SetTimer(AutoTestTimer, this, &AT4CGameMode::RunAutoTest, 2.0f, true, 3.0f);
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

		// 1) Garante uma classe com magias (Mago: Q=Fire Dart, E=FireStorm).
		if (!PS->HasChosenClass())
		{
			PS->ServerSelectClass(ET4CClass::Mage);
			UE_LOG(LogTemp, Display, TEXT("[T4C][AutoTest] %s -> classe Mago"), *PS->GetPlayerName());
			continue;
		}

		// 2) Usa Q (e E a cada 3 ticks) via o mesmo caminho do input.
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		const UT4CAttributeSet* Set = ASC ? ASC->GetSet<UT4CAttributeSet>() : nullptr;
		if (ASC && Set)
		{
			// Pressiona Q duas vezes seguidas: a 2ª deve ser bloqueada pelo cooldown.
			ASC->AbilityLocalInputPressed(static_cast<int32>(ET4CAbilityInputID::AbilityQ));
			ASC->AbilityLocalInputPressed(static_cast<int32>(ET4CAbilityInputID::AbilityQ));
			if (AutoTestTick % 3 == 0)
			{
				ASC->AbilityLocalInputPressed(static_cast<int32>(ET4CAbilityInputID::AbilityE));
			}
			UE_LOG(LogTemp, Display, TEXT("[T4C][AutoTest] %s Q x2 (Mana %.0f/%.0f, cdQ %.2fs)"),
				*PS->GetPlayerName(), Set->GetMana(), Set->GetMaxMana(),
				ASC->GetActiveEffectsTimeRemaining(FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
					FGameplayTagContainer(T4CTags::Cooldown_Q))).Num() > 0 ? 1.f : 0.f);
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
