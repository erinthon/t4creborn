#include "GAS/Effects/Exec_Damage.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CGameplayTags.h"

// Captura de atributos do alvo (sem snapshot — valores no momento do hit).
struct FT4CDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageReduction);

	FT4CDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UT4CAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UT4CAttributeSet, DamageReduction, Target, false);
	}
};

static const FT4CDamageStatics& DamageStatics()
{
	static FT4CDamageStatics Statics;
	return Statics;
}

UExec_Damage::UExec_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().DamageReductionDef);
}

void UExec_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvalParams, Armor);
	Armor = FMath::Max(0.f, Armor);

	float Reduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageReductionDef, EvalParams, Reduction);
	Reduction = FMath::Clamp(Reduction, 0.f, 0.95f);

	// Dano bruto vem do atacante via SetByCaller.
	const float RawDamage = Spec.GetSetByCallerMagnitude(T4CTags::Data_Damage, /*bWarnIfNotFound=*/false, 0.f);

	// Parry (redução fracionária) primeiro, depois armadura (plana); mín. 1.
	const float Mitigated = RawDamage * (1.f - Reduction);
	const float Final = (RawDamage > 0.f) ? FMath::Max(1.f, Mitigated - Armor) : 0.f;

	if (Final > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UT4CAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Final));
	}
}
