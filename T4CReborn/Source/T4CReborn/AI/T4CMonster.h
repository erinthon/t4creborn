#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GAS/T4CCombatant.h"
#include "Attributes/T4CAttributeData.h"
#include "T4CMonster.generated.h"

class UStaticMeshComponent;
class UT4CAbilitySystemComponent;
class UT4CAttributeSet;

/**
 * Inimigo PvE de Althea. Autoritativo no servidor; conduzido por
 * AT4CMonsterAIController. Concede XP ao jogador que o derrotar.
 * Possui seu próprio ASC (não tem PlayerState).
 */
UCLASS()
class T4CREBORN_API AT4CMonster : public ACharacter, public IAbilitySystemInterface, public IT4CCombatant
{
	GENERATED_BODY()

public:
	AT4CMonster();

	virtual void PossessedBy(AController* NewController) override;

	// --- GAS ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UT4CAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintPure, Category = "T4C")
	bool IsAlive() const;

	/** IT4CCombatant: servidor reage à morte (XP ao matador, loot, respawn). */
	virtual void HandleDeath(AActor* Killer) override;

	float GetAttackRange() const { return AttackRange; }

	/** Local onde o monstro nasceu, usado pelo GameMode para respawn. */
	FVector GetSpawnLocation() const { return SpawnLocation; }

	/** Servidor: tenta um golpe melee respeitando o cooldown. */
	void TryMeleeAttack();

protected:
	virtual void BeginPlay() override;

	/** Servidor: liga o ASC, define stats, aplica GEs de startup e enche vitais. */
	void InitAbilitySystem();

	UPROPERTY(VisibleAnywhere, Category = "T4C|GAS")
	TObjectPtr<UT4CAbilitySystemComponent> AbilitySystem;

	UPROPERTY(VisibleAnywhere, Category = "T4C|GAS")
	TObjectPtr<UT4CAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T4C")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Monster")
	FT4CPrimaryStats Stats;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Combat")
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Combat")
	float AttackCooldown = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Combat")
	float BaseDamage = 6.f;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Progression")
	int32 XPReward = 50;

private:
	float LastAttackTime = -100.f;
	bool bDeadHandled = false;
	FVector SpawnLocation = FVector::ZeroVector;
};
