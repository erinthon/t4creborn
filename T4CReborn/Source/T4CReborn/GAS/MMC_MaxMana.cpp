#include "GAS/MMC_MaxMana.h"
#include "GAS/T4CAttributeSet.h"
#include "Attributes/T4CAttributeData.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	IntelligenceDef.AttributeToCapture = UT4CAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	IntelligenceDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(IntelligenceDef);

	WisdomDef.AttributeToCapture = UT4CAttributeSet::GetWisdomAttribute();
	WisdomDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	WisdomDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(WisdomDef);
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Intelligence = 0.f;
	GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvalParams, Intelligence);
	float Wisdom = 0.f;
	GetCapturedAttributeMagnitude(WisdomDef, Spec, EvalParams, Wisdom);

	Intelligence = FMath::Max(0.f, Intelligence);
	Wisdom = FMath::Max(0.f, Wisdom);

	const FT4CBalanceConstants B;
	return B.BaseMana + Intelligence * B.ManaPerIntelligence + Wisdom * B.ManaPerWisdom;
}
