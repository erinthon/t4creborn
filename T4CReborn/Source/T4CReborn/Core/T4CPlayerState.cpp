#include "Core/T4CPlayerState.h"
#include "Player/T4CCharacter.h"
#include "GAS/T4CAbilitySystemComponent.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CAbilityInputID.h"
#include "GAS/Abilities/GA_ProjectileAttack.h"
#include "GAS/Abilities/GA_Heal.h"
#include "GAS/Abilities/GA_Parry.h"
#include "Attributes/T4CAbilityData.h"
#include "Items/T4CInventoryComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"

namespace
{
	// Classe de GameplayAbility correspondente a cada tipo de habilidade.
	TSubclassOf<UGameplayAbility> AbilityClassForKind(ET4CAbilityKind Kind)
	{
		switch (Kind)
		{
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
	// Curva simples (placeholder): cresce linearmente com o nível.
	// Substituir por DataTable de curva na Fase 2.
	return CharacterLevel * 100;
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

void AT4CPlayerState::OnRep_Progression()
{
	OnStatsChanged.Broadcast();
}
