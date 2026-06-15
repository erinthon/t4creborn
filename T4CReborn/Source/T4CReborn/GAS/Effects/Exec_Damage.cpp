#include "GAS/Effects/Exec_Damage.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CGameplayTags.h"

// Captura de atributos: armadura do alvo (sem snapshot — valor no momento do hit).
struct FT4CDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);

	FT4CDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UT4CAttributeSet, Armor, Target, false);
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

	// Dano bruto vem do atacante via SetByCaller.
	const float RawDamage = Spec.GetSetByCallerMagnitude(T4CTags::Data_Damage, /*bWarnIfNotFound=*/false, 0.f);

	// Armadura reduz de forma plana, mas o golpe sempre tira ao menos 1.
	const float Final = (RawDamage > 0.f) ? FMath::Max(1.f, RawDamage - Armor) : 0.f;

	if (Final > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UT4CAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Final));
	}
}
