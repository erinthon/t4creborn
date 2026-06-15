#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

	/** Alcance do ataque melee em cm. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float MeleeRange = 175.f;

	/** Dano base da arma desarmada/inicial. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float BaseWeaponDamage = 10.f;

	/** Cliente → servidor: solicita um ataque melee. */
	UFUNCTION(Server, Reliable)
	void ServerAttack();

	/** Servidor: reage à morte deste personagem. */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

private:
	bool bDeadHandled = false;
};
