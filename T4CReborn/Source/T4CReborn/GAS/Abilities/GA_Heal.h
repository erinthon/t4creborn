#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/T4CGameplayAbility.h"
#include "GA_Heal.generated.h"

/** Cura o conjurador (Power + WIS * 1.5) via GE_Heal. */
UCLASS()
class T4CREBORN_API UGA_Heal : public UT4CGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
