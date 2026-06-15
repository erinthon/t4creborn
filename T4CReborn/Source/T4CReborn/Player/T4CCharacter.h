#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Attributes/T4CAttributeData.h"
#include "T4CCharacter.generated.h"

class UT4CAttributeComponent;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;

/**
 * Pawn jogável de Althea. Câmera em terceira pessoa (action-RPG).
 * Autossuficiente: corpo visível e bindings de input definidos em código/config,
 * sem necessidade de Blueprint. Combate e atributos são autoritativos no servidor.
 */
UCLASS()
class T4CREBORN_API AT4CCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AT4CCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintPure, Category = "T4C")
	UT4CAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T4C")
	TObjectPtr<UT4CAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T4C")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- Input (bindings clássicos; mapeamentos em Config/DefaultInput.ini) ---
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUpAt(float Value);
	void Attack();

	// Distribuição de pontos de atributo (teclas 1-5).
	void AllocateStat(ET4CAttribute Attribute);
	void AllocStrength()     { AllocateStat(ET4CAttribute::Strength); }
	void AllocEndurance()    { AllocateStat(ET4CAttribute::Endurance); }
	void AllocAgility()      { AllocateStat(ET4CAttribute::Agility); }
	void AllocIntelligence() { AllocateStat(ET4CAttribute::Intelligence); }
	void AllocWisdom()       { AllocateStat(ET4CAttribute::Wisdom); }

	/** Dano base da arma desarmada/inicial. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float BaseWeaponDamage = 12.f;

	/** Intervalo mínimo entre ataques (s). */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackCooldown = 0.4f;

	/** Projétil disparado ao atacar. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AT4CProjectile> ProjectileClass;

	/** Cliente → servidor: solicita um ataque (dispara um projétil). */
	UFUNCTION(Server, Reliable)
	void ServerAttack();

	/** Servidor: reage à morte deste personagem. */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

private:
	bool bDeadHandled = false;
	float LastAttackTime = -100.f;
};
