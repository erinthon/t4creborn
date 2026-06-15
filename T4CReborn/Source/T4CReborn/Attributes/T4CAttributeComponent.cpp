#include "Attributes/T4CAttributeComponent.h"
#include "Net/UnrealNetwork.h"

UT4CAttributeComponent::UT4CAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UT4CAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UT4CAttributeComponent, Health);
	DOREPLIFETIME(UT4CAttributeComponent, MaxHealth);
	DOREPLIFETIME(UT4CAttributeComponent, Mana);
	DOREPLIFETIME(UT4CAttributeComponent, MaxMana);
}

void UT4CAttributeComponent::RecalculateDerivedStats(const FT4CPrimaryStats& Stats, bool bRefill)
{
	// Apenas o servidor é autoridade sobre os atributos.
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	const float PrevMaxHealth = MaxHealth;
	const float PrevMaxMana = MaxMana;

	// MaxHP = BaseHP + END * HPPerEndurance   (GDD seção 2)
	MaxHealth = Balance.BaseHP + Stats.Endurance * Balance.HPPerEndurance;
	MaxMana = Balance.BaseMana
		+ Stats.Intelligence * Balance.ManaPerIntelligence
		+ Stats.Wisdom * Balance.ManaPerWisdom;

	if (bRefill)
	{
		Health = MaxHealth;
		Mana = MaxMana;
	}
	else
	{
		// Mantém o valor atual, clamp ao novo teto; ganha o delta de HP máximo.
		Health = FMath::Clamp(Health + (MaxHealth - PrevMaxHealth), 0.f, MaxHealth);
		Mana = FMath::Clamp(Mana + (MaxMana - PrevMaxMana), 0.f, MaxMana);
	}

	// OnRep não dispara no servidor; notifica localmente para HUD do host.
	OnHealthChanged.Broadcast(this, Health, MaxHealth, nullptr);
}

float UT4CAttributeComponent::ApplyDamage(float RawDamage, AActor* InstigatorActor)
{
	if (GetOwnerRole() != ROLE_Authority || bIsDead || RawDamage <= 0.f)
	{
		return 0.f;
	}

	const float OldHealth = Health;
	Health = FMath::Clamp(Health - RawDamage, 0.f, MaxHealth);
	const float ActualDamage = OldHealth - Health;

	OnHealthChanged.Broadcast(this, Health, MaxHealth, InstigatorActor);

	if (Health <= 0.f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast(InstigatorActor);
	}

	return ActualDamage;
}

void UT4CAttributeComponent::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(this, Health, MaxHealth, nullptr);

	if (Health <= 0.f && OldHealth > 0.f)
	{
		OnDeath.Broadcast(nullptr);
	}
}

void UT4CAttributeComponent::OnRep_Mana(float OldMana)
{
	// Reservado para feedback de UI de mana.
}
