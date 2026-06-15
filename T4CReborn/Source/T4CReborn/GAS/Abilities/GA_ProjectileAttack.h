#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/T4CGameplayAbility.h"
#include "GA_ProjectileAttack.generated.h"

/** Dispara um projétil de ataque (dano escala com STR/INT/WIS e a arma equipada). */
UCLASS()
class T4CREBORN_API UGA_ProjectileAttack : public UT4CGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
