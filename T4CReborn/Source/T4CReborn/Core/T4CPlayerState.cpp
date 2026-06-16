#include "Core/T4CPlayerState.h"
#include "Player/T4CCharacter.h"
#include "GAS/T4CAbilitySystemComponent.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CAbilityInputID.h"
#include "GAS/Abilities/GA_ProjectileAttack.h"
#include "GAS/Abilities/GA_MeleeAttack.h"
#include "GAS/Abilities/GA_Heal.h"
#include "GAS/Abilities/GA_Parry.h"
#include "Attributes/T4CAbilityData.h"
#include "Items/T4CInventoryComponent.h"
#include "Core/T4CPersistenceSubsystem.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/GameInstance.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Net/UnrealNetwork.h"

namespace
{
	// Classe de GameplayAbility correspondente a cada tipo de habilidade.
	TSubclassOf<UGameplayAbility> AbilityClassForKind(ET4CAbilityKind Kind)
	{
		switch (Kind)
		{
		case ET4CAbilityKind::Melee: return UGA_MeleeAttack::StaticClass();
		case ET4CAbilityKind::Heal:  return UGA_Heal::StaticClass();
		case ET4CAbilityKind::Parry: return UGA_Parry::StaticClass();
		case ET4CAbilityKind::Projectile:
		default:                     return UGA_ProjectileAttack::StaticClass();
		}
	}
}

AT4CPlayerState::AT4CPlayerState()
{
	// Replicação mais responsiva para dados de progressão e atributos GAS.
	SetNetUpdateFrequency(30.f);

	// ASC + AttributeSet vivem no PlayerState (persistem entre respawns do pawn).
	AbilitySystem = CreateDefaultSubobject<UT4CAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystem->SetIsReplicated(true);
	AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UT4CAttributeSet>(TEXT("AttributeSet"));

	// Inventário persiste no PlayerState (não no pawn, que morre/respawna).
	Inventory = CreateDefaultSubobject<UT4CInventoryComponent>(TEXT("Inventory"));
}

UAbilitySystemComponent* AT4CPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

void AT4CPlayerState::InitializeAttributes()
{
	if (!AbilitySystem || !HasAuthority())
	{
		return;
	}
	AbilitySystem->SetPrimaryStats(PrimaryStats);
	AbilitySystem->ApplyStartupEffects(); // infinitos (derivados + regen), uma vez
	AbilitySystem->RefillVitals();        // enche HP/Mana (também no respawn)

	// Reaplica bônus do equipamento aos atributos do ASC.
	if (Inventory)
	{
		Inventory->RefreshEquipmentBonuses();
	}
}

void AT4CPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT4CPlayerState, PrimaryStats);
	DOREPLIFETIME(AT4CPlayerState, CharacterLevel);
	DOREPLIFETIME(AT4CPlayerState, Experience);
	DOREPLIFETIME(AT4CPlayerState, UnspentStatPoints);
	DOREPLIFETIME(AT4CPlayerState, UnspentSkillPoints);
	DOREPLIFETIME(AT4CPlayerState, ChosenClass);
	DOREPLIFETIME(AT4CPlayerState, bHasChosenClass);
	DOREPLIFETIME(AT4CPlayerState, Gold);
	DOREPLIFETIME(AT4CPlayerState, bLoadResolved);
}

void AT4CPlayerState::GrantGold(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return;
	}
	Gold += Amount;
	OnStatsChanged.Broadcast();
}

bool AT4CPlayerState::SpendGold(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || Gold < Amount)
	{
		return false;
	}
	Gold -= Amount;
	OnStatsChanged.Broadcast();
	return true;
}

bool AT4CPlayerState::TrainPrimaryAttribute()
{
	if (!HasAuthority() || !bHasChosenClass || UnspentSkillPoints < TrainCostSkillPoints)
	{
		return false;
	}
	UnspentSkillPoints -= TrainCostSkillPoints;
	const ET4CAttribute Attr = T4CClasses::PrimaryAttribute(ChosenClass);
	PrimaryStats.Add(Attr, 1);
	PushStatsToASC(/*bRefill=*/false);
	OnStatsChanged.Broadcast();
	UE_LOG(LogTemp, Display, TEXT("[T4C] %s treinou (+1 atributo primario, sobram %d pericia)"),
		*GetPlayerName(), UnspentSkillPoints);
	return true;
}

void AT4CPlayerState::ServerSelectClass_Implementation(ET4CClass Class)
{
	if (!HasAuthority() || bHasChosenClass)
	{
		return; // a classe é escolhida uma única vez
	}

	ChosenClass = Class;
	bHasChosenClass = true;
	PrimaryStats = T4CClasses::Get(Class).Stats;

	UE_LOG(LogTemp, Display, TEXT("[T4C] %s escolheu a classe %s"), *GetPlayerName(), *GetClassName());

	// Recalcula HP/Mana com os atributos do roll e enche as barras.
	PushStatsToASC(/*bRefill=*/true);
	GrantClassAbilities();
	OnStatsChanged.Broadcast();
}

void AT4CPlayerState::ServerResetClass_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	bHasChosenClass = false;
	CharacterLevel = 1;
	Experience = 0;
	UnspentStatPoints = 0;
	UnspentSkillPoints = 0;
	PrimaryStats = FT4CPrimaryStats(); // volta aos 10 padrão

	UE_LOG(LogTemp, Display, TEXT("[T4C] %s recomeçou (escolher classe)"), *GetPlayerName());

	ClearAbilities();
	PushStatsToASC(/*bRefill=*/true);
	OnStatsChanged.Broadcast();
}

int32 AT4CPlayerState::GetXPForNextLevel() const
{
	// Curva crescente (triangular): cada nível custa progressivamente mais.
	// N1=100, N2=300, N3=600, N4=1000... Mantém inteiro e previsível.
	return 50 * CharacterLevel * (CharacterLevel + 1);
}

void AT4CPlayerState::GrantExperience(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return;
	}

	Experience += Amount;

	while (Experience >= GetXPForNextLevel())
	{
		Experience -= GetXPForNextLevel();
		LevelUp();
	}

	OnStatsChanged.Broadcast();
}

void AT4CPlayerState::LevelUp()
{
	// Pré-condição: chamado apenas no servidor por GrantExperience.
	CharacterLevel++;
	UnspentStatPoints += StatPointsPerLevel;
	UnspentSkillPoints += SkillPointsPerLevel;

	UE_LOG(LogTemp, Display, TEXT("[T4C] %s subiu para o nivel %d (+%d atributo / +%d pericia)"),
		*GetPlayerName(), CharacterLevel, StatPointsPerLevel, SkillPointsPerLevel);

	// HP é concedido no level-up conforme END atual (fiel ao T4C).
	PushStatsToASC(/*bRefill=*/false);
}

void AT4CPlayerState::ServerAllocateStat_Implementation(ET4CAttribute Attribute)
{
	if (!HasAuthority() || UnspentStatPoints <= 0)
	{
		return;
	}

	PrimaryStats.Add(Attribute, 1);
	UnspentStatPoints--;

	PushStatsToASC(/*bRefill=*/false);
	OnStatsChanged.Broadcast();
}

void AT4CPlayerState::PushStatsToASC(bool bRefill)
{
	if (!AbilitySystem || !HasAuthority())
	{
		return;
	}
	// Atualiza os 5 atributos base; os MMCs recomputam MaxHealth/MaxMana de forma
	// reativa. RefillVitals só quando pedido (ex.: troca de classe).
	AbilitySystem->SetPrimaryStats(PrimaryStats);
	if (bRefill)
	{
		AbilitySystem->RefillVitals();
	}
}

void AT4CPlayerState::GrantClassAbilities()
{
	if (!HasAuthority() || !AbilitySystem)
	{
		return;
	}
	ClearAbilities();

	for (int32 Slot = 0; Slot < 2; ++Slot)
	{
		const FT4CAbility Data = T4CAbilities::Get(ChosenClass, Slot);
		const int32 InputID = (Slot == 0)
			? static_cast<int32>(ET4CAbilityInputID::AbilityQ)
			: static_cast<int32>(ET4CAbilityInputID::AbilityE);

		FGameplayAbilitySpec Spec(AbilityClassForKind(Data.Kind), /*Level=*/1, InputID, /*SourceObject=*/this);
		GrantedAbilities.Add(AbilitySystem->GiveAbility(Spec));
	}
}

void AT4CPlayerState::ClearAbilities()
{
	if (!AbilitySystem)
	{
		GrantedAbilities.Reset();
		return;
	}
	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilities)
	{
		AbilitySystem->ClearAbility(Handle);
	}
	GrantedAbilities.Reset();
}

FString AT4CPlayerState::GetSaveSlotName() const
{
	// Sanitiza o nome do jogador para um nome de arquivo seguro.
	FString Clean;
	for (const TCHAR C : GetPlayerName())
	{
		if (FChar::IsAlnum(C))
		{
			Clean.AppendChar(C);
		}
	}
	if (Clean.IsEmpty())
	{
		Clean = TEXT("Player");
	}
	return FString::Printf(TEXT("T4C_%s"), *Clean);
}

static UT4CPersistenceSubsystem* GetPersistence(const AActor* Actor)
{
	if (Actor)
	{
		if (const UWorld* World = Actor->GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				return GI->GetSubsystem<UT4CPersistenceSubsystem>();
			}
		}
	}
	return nullptr;
}

void AT4CPlayerState::SaveCharacter()
{
	// Só persiste personagens que já escolheram classe (evita salvar vazio).
	if (!HasAuthority() || !bHasChosenClass)
	{
		return;
	}
	if (UT4CPersistenceSubsystem* Persistence = GetPersistence(this))
	{
		Persistence->SaveCharacter(GetSaveSlotName(), SerializeToJson());
	}
}

void AT4CPlayerState::LoadCharacterOnce()
{
	if (!HasAuthority() || bSaveLoaded)
	{
		return;
	}
	bSaveLoaded = true;

	if (UT4CPersistenceSubsystem* Persistence = GetPersistence(this))
	{
		// Assíncrono: o HUD mostra "carregando" até o callback resolver.
		Persistence->LoadCharacter(GetSaveSlotName(),
			FT4COnCharacterLoaded::CreateUObject(this, &AT4CPlayerState::OnCharacterLoaded));
	}
	else
	{
		// Sem subsystem de persistência: resolve imediatamente (vai ao menu de classe).
		bLoadResolved = true;
		OnStatsChanged.Broadcast();
	}
}

void AT4CPlayerState::OnCharacterLoaded(bool bFound, const FString& Json)
{
	if (bFound)
	{
		ApplyFromJson(Json);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("[T4C] Sem save para %s (personagem novo)"), *GetPlayerName());
	}
	bLoadResolved = true; // libera o HUD (menu de classe ou jogo)
	OnStatsChanged.Broadcast();
}

FString AT4CPlayerState::SerializeToJson() const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("version"), 1);
	Root->SetBoolField(TEXT("hasChosenClass"), bHasChosenClass);
	Root->SetNumberField(TEXT("class"), static_cast<int32>(ChosenClass));

	const TSharedRef<FJsonObject> Stats = MakeShared<FJsonObject>();
	Stats->SetNumberField(TEXT("str"), PrimaryStats.Strength);
	Stats->SetNumberField(TEXT("end"), PrimaryStats.Endurance);
	Stats->SetNumberField(TEXT("agi"), PrimaryStats.Agility);
	Stats->SetNumberField(TEXT("int"), PrimaryStats.Intelligence);
	Stats->SetNumberField(TEXT("wis"), PrimaryStats.Wisdom);
	Root->SetObjectField(TEXT("stats"), Stats);

	Root->SetNumberField(TEXT("level"), CharacterLevel);
	Root->SetNumberField(TEXT("xp"), Experience);
	Root->SetNumberField(TEXT("unspentStat"), UnspentStatPoints);
	Root->SetNumberField(TEXT("unspentSkill"), UnspentSkillPoints);
	Root->SetNumberField(TEXT("gold"), Gold);

	int32 WeaponIdx = -1, ArmorIdx = -1;
	TArray<TSharedPtr<FJsonValue>> ItemsJson;
	if (Inventory)
	{
		WeaponIdx = Inventory->GetEquippedWeaponIndex();
		ArmorIdx = Inventory->GetEquippedArmorIndex();
		for (const FT4CItem& Item : Inventory->GetItems())
		{
			const TSharedRef<FJsonObject> J = MakeShared<FJsonObject>();
			J->SetStringField(TEXT("id"), Item.Id.ToString());
			J->SetStringField(TEXT("name"), Item.Name);
			J->SetNumberField(TEXT("type"), static_cast<int32>(Item.Type));
			J->SetNumberField(TEXT("weaponDamage"), Item.WeaponDamage);
			J->SetNumberField(TEXT("armor"), Item.Armor);
			J->SetNumberField(TEXT("heal"), Item.HealAmount);
			J->SetNumberField(TEXT("rarity"), Item.Rarity);
			ItemsJson.Add(MakeShared<FJsonValueObject>(J));
		}
	}
	Root->SetNumberField(TEXT("equippedWeapon"), WeaponIdx);
	Root->SetNumberField(TEXT("equippedArmor"), ArmorIdx);
	Root->SetArrayField(TEXT("items"), ItemsJson);

	FString Out;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
	FJsonSerializer::Serialize(Root, Writer);
	return Out;
}

void AT4CPlayerState::ApplyFromJson(const FString& Json)
{
	if (!HasAuthority())
	{
		return;
	}
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid() || !Root->GetBoolField(TEXT("hasChosenClass")))
	{
		return;
	}

	ChosenClass = static_cast<ET4CClass>(Root->GetIntegerField(TEXT("class")));
	bHasChosenClass = true;

	const TSharedPtr<FJsonObject>* Stats;
	if (Root->TryGetObjectField(TEXT("stats"), Stats))
	{
		PrimaryStats.Strength = (*Stats)->GetIntegerField(TEXT("str"));
		PrimaryStats.Endurance = (*Stats)->GetIntegerField(TEXT("end"));
		PrimaryStats.Agility = (*Stats)->GetIntegerField(TEXT("agi"));
		PrimaryStats.Intelligence = (*Stats)->GetIntegerField(TEXT("int"));
		PrimaryStats.Wisdom = (*Stats)->GetIntegerField(TEXT("wis"));
	}
	CharacterLevel = Root->GetIntegerField(TEXT("level"));
	Experience = Root->GetIntegerField(TEXT("xp"));
	UnspentStatPoints = Root->GetIntegerField(TEXT("unspentStat"));
	UnspentSkillPoints = Root->GetIntegerField(TEXT("unspentSkill"));
	Gold = Root->HasField(TEXT("gold")) ? Root->GetIntegerField(TEXT("gold")) : 0;

	TArray<FT4CItem> Items;
	const TArray<TSharedPtr<FJsonValue>>* ItemsArr;
	if (Root->TryGetArrayField(TEXT("items"), ItemsArr))
	{
		for (const TSharedPtr<FJsonValue>& V : *ItemsArr)
		{
			const TSharedPtr<FJsonObject> J = V->AsObject();
			if (!J.IsValid())
			{
				continue;
			}
			FT4CItem Item;
			Item.Id = FName(*J->GetStringField(TEXT("id")));
			Item.Name = J->GetStringField(TEXT("name"));
			Item.Type = static_cast<ET4CItemType>(J->GetIntegerField(TEXT("type")));
			Item.WeaponDamage = J->GetNumberField(TEXT("weaponDamage"));
			Item.Armor = J->GetNumberField(TEXT("armor"));
			Item.HealAmount = J->GetNumberField(TEXT("heal"));
			Item.Rarity = J->GetIntegerField(TEXT("rarity"));
			Items.Add(Item);
		}
	}
	const int32 WeaponIdx = Root->GetIntegerField(TEXT("equippedWeapon"));
	const int32 ArmorIdx = Root->GetIntegerField(TEXT("equippedArmor"));

	PushStatsToASC(/*bRefill=*/true);
	GrantClassAbilities();
	if (Inventory)
	{
		Inventory->RestoreFromSave(Items, WeaponIdx, ArmorIdx);
	}
	OnStatsChanged.Broadcast();

	UE_LOG(LogTemp, Display, TEXT("[T4C] Carregou %s (classe %s, nivel %d, %d itens)"),
		*GetPlayerName(), *GetClassName(), CharacterLevel, Items.Num());
}

void AT4CPlayerState::OnRep_Progression()
{
	OnStatsChanged.Broadcast();
}
