#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxMana.generated.h"

/**
 * Calcula MaxMana = BaseMana + Intelligence * ManaPerInt + Wisdom * ManaPerWisdom.
 * Captura INT e WIS sem snapshot → reavalia quando esses atributos mudam.
 */
UCLASS()
class T4CREBORN_API UMMC_MaxMana : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_MaxMana();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
	FGameplayEffectAttributeCaptureDefinition WisdomDef;
};
