#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "T4CAttributeSet.generated.h"

// Gera Getter/Setter/Initter padrão de cada atributo (idioma do GAS).
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * AttributeSet de T4C: Reborn. Centraliza os atributos numéricos do combate.
 * Vive no ASC (PlayerState para jogadores, pawn para monstros). Só o servidor
 * é autoridade; clientes recebem via replicação (REPNOTIFY_Always p/ predição).
 *
 * - Primários (STR/END/AGI/INT/WIS): fonte do roll/progressão.
 * - Vitais (Health/MaxHealth/Mana/MaxMana): MaxHealth/MaxMana derivados via MMC.
 * - Combate (Armor/WeaponDamageBonus): vêm do equipamento.
 * - Meta (IncomingDamage/IncomingHealing): transientes, não replicados; a
 *   mitigação por armadura ocorre em UExec_Damage e o resultado entra aqui.
 */
UCLASS()
class T4CREBORN_API UT4CAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UT4CAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// --- Atributos primários ---
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "T4C|Primary")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, Strength)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Endurance, Category = "T4C|Primary")
	FGameplayAttributeData Endurance;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, Endurance)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Agility, Category = "T4C|Primary")
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, Agility)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "T4C|Primary")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, Intelligence)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Wisdom, Category = "T4C|Primary")
	FGameplayAttributeData Wisdom;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, Wisdom)

	// --- Vitais ---
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "T4C|Vitals")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "T4C|Vitals")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "T4C|Vitals")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "T4C|Vitals")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, MaxMana)

	// --- Combate (equipamento) ---
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "T4C|Combat")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, Armor)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponDamageBonus, Category = "T4C|Combat")
	FGameplayAttributeData WeaponDamageBonus;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, WeaponDamageBonus)

	/** Redução fracionária de dano recebido (0..1), concedida pelo Parry. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageReduction, Category = "T4C|Combat")
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, DamageReduction)

	// --- Meta-atributos (transientes, não replicados) ---
	UPROPERTY(BlueprintReadOnly, Category = "T4C|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category = "T4C|Meta")
	FGameplayAttributeData IncomingHealing;
	ATTRIBUTE_ACCESSORS(UT4CAttributeSet, IncomingHealing)

	bool IsAlive() const { return GetHealth() > 0.f; }

	/** Servidor: zera o estado de morte (usado no respawn/refill). */
	void ResetDeathState() { bDeathHandled = false; }

protected:
	UFUNCTION() void OnRep_Strength(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, Strength, Old); }
	UFUNCTION() void OnRep_Endurance(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, Endurance, Old); }
	UFUNCTION() void OnRep_Agility(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, Agility, Old); }
	UFUNCTION() void OnRep_Intelligence(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, Intelligence, Old); }
	UFUNCTION() void OnRep_Wisdom(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, Wisdom, Old); }
	UFUNCTION() void OnRep_Health(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, Health, Old); }
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, MaxHealth, Old); }
	UFUNCTION() void OnRep_Mana(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, Mana, Old); }
	UFUNCTION() void OnRep_MaxMana(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, MaxMana, Old); }
	UFUNCTION() void OnRep_Armor(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, Armor, Old); }
	UFUNCTION() void OnRep_WeaponDamageBonus(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, WeaponDamageBonus, Old); }
	UFUNCTION() void OnRep_DamageReduction(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(UT4CAttributeSet, DamageReduction, Old); }

private:
	/** Mantém o valor atual proporcional quando o máximo muda (ex.: level-up). */
	void AdjustForMaxChange(const FGameplayAttributeData& AffectedAttr, const FGameplayAttributeData& MaxAttr,
		float NewMaxValue, const FGameplayAttribute& AffectedProperty);

	bool bDeathHandled = false;
};
