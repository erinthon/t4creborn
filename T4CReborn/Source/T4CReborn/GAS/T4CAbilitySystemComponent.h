#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "T4CAbilitySystemComponent.generated.h"

struct FT4CPrimaryStats;

/**
 * ASC de T4C: Reborn. Subclasse fina com helpers de inicialização para
 * centralizar a sequência correta: setar atributos base → aplicar GEs infinitos
 * (derivados + regen) → encher vitais. Todos os helpers são servidor-apenas.
 */
UCLASS()
class T4CREBORN_API UT4CAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UT4CAbilitySystemComponent();

	/** Servidor: define os 5 atributos primários base a partir do roll/stats. */
	void SetPrimaryStats(const FT4CPrimaryStats& Stats);

	/** Servidor: aplica os GEs infinitos (derivados + regen) uma única vez. */
	void ApplyStartupEffects();

	/** Servidor: enche HP/Mana ao máximo e limpa o estado de morte. */
	void RefillVitals();

	/** Servidor: define os bônus de equipamento (Armadura/Dano de arma base). */
	void SetEquipmentBonuses(float ArmorValue, float WeaponDamageBonus);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "T4C|GAS")
	TSubclassOf<UGameplayEffect> DerivedAttributesEffect;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|GAS")
	TSubclassOf<UGameplayEffect> RegenEffect;

private:
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass);

	bool bStartupEffectsApplied = false;
};
