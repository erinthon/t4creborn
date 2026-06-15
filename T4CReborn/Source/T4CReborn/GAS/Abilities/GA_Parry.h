#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/T4CGameplayAbility.h"
#include "GA_Parry.generated.h"

/** Concede mitigação temporária (GE_Parry, duração = Data.Duration). */
UCLASS()
class T4CREBORN_API UGA_Parry : public UT4CGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
