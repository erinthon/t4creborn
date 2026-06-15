#include "Core/T4CPlayerState.h"
#include "Player/T4CCharacter.h"
#include "GAS/T4CAbilitySystemComponent.h"
#include "GAS/T4CAttributeSet.h"
#include "Items/T4CInventoryComponent.h"
#include "Net/UnrealNetwork.h"

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

void AT4CPlayerState::OnRep_Progression()
{
	OnStatsChanged.Broadcast();
}
