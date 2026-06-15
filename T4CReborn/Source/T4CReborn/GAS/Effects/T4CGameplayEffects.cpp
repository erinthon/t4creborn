#include "GAS/Effects/T4CGameplayEffects.h"
#include "GAS/Effects/Exec_Damage.h"
#include "GAS/MMC_MaxHealth.h"
#include "GAS/MMC_MaxMana.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CGameplayTags.h"
#include "Attributes/T4CAttributeData.h"

UGE_DerivedAttributes::UGE_DerivedAttributes()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// MaxHealth (via MMC, reativo a Endurance).
	FCustomCalculationBasedFloat MaxHPCalc;
	MaxHPCalc.CalculationClassMagnitude = UMMC_MaxHealth::StaticClass();
	FGameplayModifierInfo MaxHPMod;
	MaxHPMod.Attribute = UT4CAttributeSet::GetMaxHealthAttribute();
	MaxHPMod.ModifierOp = EGameplayModOp::Additive;
	MaxHPMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(MaxHPCalc);
	Modifiers.Add(MaxHPMod);

	// MaxMana (via MMC, reativo a INT/WIS).
	FCustomCalculationBasedFloat MaxManaCalc;
	MaxManaCalc.CalculationClassMagnitude = UMMC_MaxMana::StaticClass();
	FGameplayModifierInfo MaxManaMod;
	MaxManaMod.Attribute = UT4CAttributeSet::GetMaxManaAttribute();
	MaxManaMod.ModifierOp = EGameplayModOp::Additive;
	MaxManaMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(MaxManaCalc);
	Modifiers.Add(MaxManaMod);
}

UGE_Regen::UGE_Regen()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(0.25f);

	const FT4CBalanceConstants B;

	FGameplayModifierInfo HealthMod;
	HealthMod.Attribute = UT4CAttributeSet::GetHealthAttribute();
	HealthMod.ModifierOp = EGameplayModOp::Additive;
	HealthMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(B.HealthRegenPerSec * 0.25f));
	Modifiers.Add(HealthMod);

	FGameplayModifierInfo ManaMod;
	ManaMod.Attribute = UT4CAttributeSet::GetManaAttribute();
	ManaMod.ModifierOp = EGameplayModOp::Additive;
	ManaMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(B.ManaRegenPerSec * 0.25f));
	Modifiers.Add(ManaMod);
}

UGE_Damage::UGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = UExec_Damage::StaticClass();
	Executions.Add(ExecDef);
}

UGE_Heal::UGE_Heal()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat HealSBC;
	HealSBC.DataTag = T4CTags::Data_Healing;

	FGameplayModifierInfo HealMod;
	HealMod.Attribute = UT4CAttributeSet::GetIncomingHealingAttribute();
	HealMod.ModifierOp = EGameplayModOp::Additive;
	HealMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(HealSBC);
	Modifiers.Add(HealMod);
}

UGE_Cost::UGE_Cost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat CostSBC;
	CostSBC.DataTag = T4CTags::Data_ManaCost;

	FGameplayModifierInfo ManaMod;
	ManaMod.Attribute = UT4CAttributeSet::GetManaAttribute();
	ManaMod.ModifierOp = EGameplayModOp::Additive; // caller passa valor negativo
	ManaMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(CostSBC);
	Modifiers.Add(ManaMod);
}

UGE_Parry::UGE_Parry()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat DurSBC;
	DurSBC.DataTag = T4CTags::Data_Duration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(DurSBC);

	// Bônus plano de armadura enquanto ativo (mitigação temporária).
	FGameplayModifierInfo ArmorMod;
	ArmorMod.Attribute = UT4CAttributeSet::GetArmorAttribute();
	ArmorMod.ModifierOp = EGameplayModOp::Additive;
	ArmorMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(30.f));
	Modifiers.Add(ArmorMod);
}
