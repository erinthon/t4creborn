#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CCombatant.h"
#include "GAS/T4CGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UT4CAttributeSet::UT4CAttributeSet()
{
}

void UT4CAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, Endurance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, Agility, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, Wisdom, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, WeaponDamageBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UT4CAttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
}

void UT4CAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		AdjustForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetDamageReductionAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 0.95f);
	}
}

void UT4CAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attr = Data.EvaluatedData.Attribute;

	if (Attr == GetIncomingDamageAttribute())
	{
		const float Dmg = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (Dmg > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - Dmg, 0.f, GetMaxHealth()));
		}
	}
	else if (Attr == GetIncomingHealingAttribute())
	{
		const float Heal = GetIncomingHealing();
		SetIncomingHealing(0.f);
		if (Heal > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() + Heal, 0.f, GetMaxHealth()));
		}
	}
	else if (Attr == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Attr == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}

	// Morte: dispara uma única vez; despacha ao avatar via IT4CCombatant.
	if (GetHealth() <= 0.f && !bDeathHandled)
	{
		bDeathHandled = true;

		AActor* Killer = Data.EffectSpec.GetContext().GetInstigator();
		AActor* Avatar = GetOwningAbilitySystemComponent() ? GetOwningAbilitySystemComponent()->GetAvatarActor() : nullptr;

		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			ASC->AddLooseGameplayTag(T4CTags::State_Dead);
		}
		if (IT4CCombatant* Combatant = Cast<IT4CCombatant>(Avatar))
		{
			Combatant->HandleDeath(Killer);
		}
	}
}

void UT4CAttributeSet::AdjustForMaxChange(const FGameplayAttributeData& AffectedAttr, const FGameplayAttributeData& MaxAttr,
	float NewMaxValue, const FGameplayAttribute& AffectedProperty)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	const float CurrentMax = MaxAttr.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMax, NewMaxValue) && ASC)
	{
		// Mantém a proporção atual: HP% constante ao subir o teto (level-up).
		const float CurrentValue = AffectedAttr.GetCurrentValue();
		const float NewDelta = (CurrentMax > 0.f)
			? (CurrentValue * NewMaxValue / CurrentMax - CurrentValue)
			: NewMaxValue;
		ASC->ApplyModToAttribute(AffectedProperty, EGameplayModOp::Additive, NewDelta);
	}
}
