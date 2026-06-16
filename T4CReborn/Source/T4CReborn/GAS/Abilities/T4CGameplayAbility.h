#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Attributes/T4CAbilityData.h"
#include "T4CGameplayAbility.generated.h"

/**
 * Base das habilidades de T4C. Data-driven: os parâmetros (poder, custo,
 * cooldown, duração) vêm de T4CAbilities::Get(classe, slot) — a mesma tabela do
 * sistema antigo. O slot é derivado do InputID guardado no spec.
 *
 * Custo e cooldown são GameplayEffects (SetByCaller): ApplyCost/ApplyCooldown
 * são sobrescritos para injetar a magnitude por habilidade. A tag de cooldown do
 * slot é adicionada dinamicamente, então um único GE serve aos dois slots.
 */
UCLASS(Abstract)
class T4CREBORN_API UT4CGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UT4CGameplayAbility();

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	/** Slot deste spec (0 = Q, 1 = E), derivado do InputID. */
	int32 GetSlot(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle) const;

	/** Tag de cooldown do slot (Cooldown.Q / Cooldown.E). */
	FGameplayTag GetSlotCooldownTag(int32 Slot) const;

	/** Parâmetros da habilidade para (classe do dono, slot). */
	FT4CAbility GetAbilityData(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle) const;

	/** Aplica a si mesmo um GE com magnitude SetByCaller (servidor). */
	void ApplySelfEffectSetByCaller(const FGameplayAbilityActorInfo* ActorInfo,
		TSubclassOf<UGameplayEffect> EffectClass, FGameplayTag DataTag, float Magnitude) const;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Ability")
	TSubclassOf<UGameplayEffect> CostEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Ability")
	TSubclassOf<UGameplayEffect> CooldownEffectClass;
};
