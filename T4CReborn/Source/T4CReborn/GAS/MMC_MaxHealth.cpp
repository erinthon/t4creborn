#include "GAS/MMC_MaxHealth.h"
#include "GAS/T4CAttributeSet.h"
#include "Attributes/T4CAttributeData.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	EnduranceDef.AttributeToCapture = UT4CAttributeSet::GetEnduranceAttribute();
	EnduranceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	EnduranceDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(EnduranceDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Endurance = 0.f;
	GetCapturedAttributeMagnitude(EnduranceDef, Spec, EvalParams, Endurance);
	Endurance = FMath::Max(0.f, Endurance);

	// Constantes de balanceamento (mesmos defaults do sistema antigo).
	const FT4CBalanceConstants B;
	return B.BaseHP + Endurance * B.HPPerEndurance;
}
