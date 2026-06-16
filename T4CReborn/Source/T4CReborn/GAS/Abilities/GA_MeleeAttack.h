#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/T4CGameplayAbility.h"
#include "GA_MeleeAttack.generated.h"

/** Golpe de proximidade: sweep curto à frente; exige estar perto do alvo. */
UCLASS()
class T4CREBORN_API UGA_MeleeAttack : public UT4CGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
