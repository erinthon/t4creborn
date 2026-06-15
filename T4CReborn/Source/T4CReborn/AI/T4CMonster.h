#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Attributes/T4CAttributeData.h"
#include "T4CMonster.generated.h"

class UT4CAttributeComponent;
class UStaticMeshComponent;

/**
 * Inimigo PvE de Althea. Autoritativo no servidor; conduzido por
 * AT4CMonsterAIController. Concede XP ao jogador que o derrotar.
 */
UCLASS()
class T4CREBORN_API AT4CMonster : public ACharacter
{
	GENERATED_BODY()

public:
	AT4CMonster();

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintPure, Category = "T4C")
	UT4CAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

	float GetAttackRange() const { return AttackRange; }

	/** Local onde o monstro nasceu, usado pelo GameMode para respawn. */
	FVector GetSpawnLocation() const { return SpawnLocation; }

	/** Servidor: tenta um golpe melee respeitando o cooldown. */
	void TryMeleeAttack();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T4C")
	TObjectPtr<UT4CAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T4C")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Monster")
	FT4CPrimaryStats Stats;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Combat")
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Combat")
	float AttackCooldown = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Combat")
	float BaseDamage = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "T4C|Progression")
	int32 XPReward = 50;

	UFUNCTION()
	void HandleDeath(AActor* Killer);

private:
	float LastAttackTime = -100.f;
	bool bDeadHandled = false;
	FVector SpawnLocation = FVector::ZeroVector;
};
