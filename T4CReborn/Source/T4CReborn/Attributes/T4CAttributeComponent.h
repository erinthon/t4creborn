#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Attributes/T4CAttributeData.h"
#include "T4CAttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnHealthChanged, UT4CAttributeComponent*, Component, float, NewHealth, float, MaxHealth, AActor*, InstigatorActor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);

/**
 * Estado de combate em tempo real (HP/Mana) e estatísticas derivadas.
 * Autoridade: SOMENTE o servidor escreve. Clientes recebem via OnRep.
 * Os valores Max são derivados dos 5 atributos primários (ver GDD seção 2).
 */
UCLASS(ClassGroup = (T4C), meta = (BlueprintSpawnableComponent))
class T4CREBORN_API UT4CAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT4CAttributeComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Recalcula MaxHealth/MaxMana a partir dos atributos. Servidor apenas. */
	void RecalculateDerivedStats(const FT4CPrimaryStats& Stats, bool bRefill);

	/** Aplica dano ao alvo. Retorna o dano efetivamente aplicado. Servidor apenas. */
	float ApplyDamage(float RawDamage, AActor* InstigatorActor);

	UFUNCTION(BlueprintPure, Category = "T4C|Attributes")
	bool IsAlive() const { return Health > 0.f; }

	UFUNCTION(BlueprintPure, Category = "T4C|Attributes")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "T4C|Attributes")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "T4C|Attributes")
	float GetMana() const { return Mana; }

	UFUNCTION(BlueprintPure, Category = "T4C|Attributes")
	float GetMaxMana() const { return MaxMana; }

	UPROPERTY(BlueprintAssignable, Category = "T4C|Attributes")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "T4C|Attributes")
	FOnDeath OnDeath;

	/** Constantes de balanceamento (idealmente vindas de DataTable). */
	UPROPERTY(EditDefaultsOnly, Category = "T4C|Balance")
	FT4CBalanceConstants Balance;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "T4C|Attributes")
	float Health = 100.f;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "T4C|Attributes")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Mana, VisibleAnywhere, Category = "T4C|Attributes")
	float Mana = 50.f;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "T4C|Attributes")
	float MaxMana = 50.f;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION()
	void OnRep_Mana(float OldMana);

private:
	bool bIsDead = false;
};
