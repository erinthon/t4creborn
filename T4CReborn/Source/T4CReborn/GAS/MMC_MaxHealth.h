#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

/**
 * Calcula MaxHealth = BaseHP + Endurance * HPPerEndurance.
 * Captura Endurance sem snapshot (bSnapshot=false) → o GE infinito que usa este
 * MMC reavalia MaxHealth automaticamente quando Endurance muda (level-up).
 */
UCLASS()
class T4CREBORN_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_MaxHealth();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
};
