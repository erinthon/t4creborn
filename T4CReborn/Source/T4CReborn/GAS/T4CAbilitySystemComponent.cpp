#include "GAS/T4CAbilitySystemComponent.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CGameplayTags.h"
#include "GAS/Effects/T4CGameplayEffects.h"
#include "Attributes/T4CAttributeData.h"

UT4CAbilitySystemComponent::UT4CAbilitySystemComponent()
{
	DerivedAttributesEffect = UGE_DerivedAttributes::StaticClass();
	RegenEffect = UGE_Regen::StaticClass();
	EquipmentEffect = UGE_Equipment::StaticClass();
}

void UT4CAbilitySystemComponent::SetPrimaryStats(const FT4CPrimaryStats& Stats)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}
	SetNumericAttributeBase(UT4CAttributeSet::GetStrengthAttribute(), Stats.Strength);
	SetNumericAttributeBase(UT4CAttributeSet::GetEnduranceAttribute(), Stats.Endurance);
	SetNumericAttributeBase(UT4CAttributeSet::GetAgilityAttribute(), Stats.Agility);
	SetNumericAttributeBase(UT4CAttributeSet::GetIntelligenceAttribute(), Stats.Intelligence);
	SetNumericAttributeBase(UT4CAttributeSet::GetWisdomAttribute(), Stats.Wisdom);
}

void UT4CAbilitySystemComponent::ApplyStartupEffects()
{
	if (!IsOwnerActorAuthoritative() || bStartupEffectsApplied)
	{
		return;
	}
	bStartupEffectsApplied = true;
	ApplyEffectToSelf(DerivedAttributesEffect);
	ApplyEffectToSelf(RegenEffect);
}

void UT4CAbilitySystemComponent::RefillVitals()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}
	SetNumericAttributeBase(UT4CAttributeSet::GetHealthAttribute(),
		GetNumericAttribute(UT4CAttributeSet::GetMaxHealthAttribute()));
	SetNumericAttributeBase(UT4CAttributeSet::GetManaAttribute(),
		GetNumericAttribute(UT4CAttributeSet::GetMaxManaAttribute()));

	if (const UT4CAttributeSet* Set = GetSet<UT4CAttributeSet>())
	{
		const_cast<UT4CAttributeSet*>(Set)->ResetDeathState();
	}
	// Idempotente: zera a contagem sem avisar quando a tag não está presente.
	SetLooseGameplayTagCount(T4CTags::State_Dead, 0);
}

void UT4CAbilitySystemComponent::ApplyEquipment(float ArmorValue, float WeaponDamageBonus)
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	// Remove o GE de equipamento anterior antes de reaplicar com os novos bônus.
	if (EquipmentEffectHandle.IsValid())
	{
		RemoveActiveGameplayEffect(EquipmentEffectHandle);
		EquipmentEffectHandle.Invalidate();
	}

	if ((ArmorValue <= 0.f && WeaponDamageBonus <= 0.f) || !EquipmentEffect)
	{
		return; // nada equipado
	}

	FGameplayEffectContextHandle Ctx = MakeEffectContext();
	Ctx.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(EquipmentEffect, 1.f, Ctx);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(T4CTags::Data_ArmorBonus, FMath::Max(0.f, ArmorValue));
		Spec.Data->SetSetByCallerMagnitude(T4CTags::Data_WeaponBonus, FMath::Max(0.f, WeaponDamageBonus));
		EquipmentEffectHandle = ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}

void UT4CAbilitySystemComponent::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!EffectClass)
	{
		return;
	}
	FGameplayEffectContextHandle Context = MakeEffectContext();
	Context.AddSourceObject(this);
	const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(EffectClass, 1.f, Context);
	if (Spec.IsValid())
	{
		ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}
