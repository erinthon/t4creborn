#include "Core/T4CPlayerState.h"
#include "Player/T4CCharacter.h"
#include "Attributes/T4CAttributeComponent.h"
#include "Net/UnrealNetwork.h"

AT4CPlayerState::AT4CPlayerState()
{
	// Replicação mais responsiva para dados de progressão.
	SetNetUpdateFrequency(10.f);
}

void AT4CPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT4CPlayerState, PrimaryStats);
	DOREPLIFETIME(AT4CPlayerState, CharacterLevel);
	DOREPLIFETIME(AT4CPlayerState, Experience);
	DOREPLIFETIME(AT4CPlayerState, UnspentStatPoints);
	DOREPLIFETIME(AT4CPlayerState, UnspentSkillPoints);
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
	PushDerivedStatsToPawn(/*bRefill=*/false);
}

void AT4CPlayerState::ServerAllocateStat_Implementation(ET4CAttribute Attribute)
{
	if (!HasAuthority() || UnspentStatPoints <= 0)
	{
		return;
	}

	PrimaryStats.Add(Attribute, 1);
	UnspentStatPoints--;

	PushDerivedStatsToPawn(/*bRefill=*/false);
	OnStatsChanged.Broadcast();
}

void AT4CPlayerState::PushDerivedStatsToPawn(bool bRefill)
{
	if (AT4CCharacter* Character = Cast<AT4CCharacter>(GetPawn()))
	{
		if (UT4CAttributeComponent* Attributes = Character->GetAttributeComponent())
		{
			Attributes->RecalculateDerivedStats(PrimaryStats, bRefill);
		}
	}
}

void AT4CPlayerState::OnRep_Progression()
{
	OnStatsChanged.Broadcast();
}
