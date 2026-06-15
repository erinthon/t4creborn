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

	// Distribuição de pontos de atributo (teclas 1-5, após escolher classe).
	void AllocateStat(ET4CAttribute Attribute);
	void AllocStrength()     { AllocateStat(ET4CAttribute::Strength); }
	void AllocEndurance()    { AllocateStat(ET4CAttribute::Endurance); }
	void AllocAgility()      { AllocateStat(ET4CAttribute::Agility); }
	void AllocIntelligence() { AllocateStat(ET4CAttribute::Intelligence); }
	void AllocWisdom()       { AllocateStat(ET4CAttribute::Wisdom); }

	// Seleção de classe (teclas 1-8, antes de escolher a classe).
	void SelectClass(int32 Index);
	void SelectClass0() { SelectClass(0); }
	void SelectClass1() { SelectClass(1); }
	void SelectClass2() { SelectClass(2); }
	void SelectClass3() { SelectClass(3); }
	void SelectClass4() { SelectClass(4); }
	void SelectClass5() { SelectClass(5); }
	void SelectClass6() { SelectClass(6); }
	void SelectClass7() { SelectClass(7); }

	/** Recomeçar: volta ao menu de classes (tecla R). */
	void ResetClass();

	/** Dano base usado pelas habilidades de projétil. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float BaseWeaponDamage = 12.f;

	/** Projétil disparado pelas habilidades. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AT4CProjectile> ProjectileClass;

	// Habilidades de classe (Q = slot 0, E = slot 1).
	void UseAbility0() { UseAbility(0); }
	void UseAbility1() { UseAbility(1); }
	void UseAbility(int32 Slot);

	UFUNCTION(Server, Reliable)
	void ServerUseAbility(int32 Slot);

public:
	/** Para o HUD: nome da habilidade no slot (0=Q, 1=E). */
	FString GetAbilityName(int32 Slot) const;

	/** Para o HUD: segundos de cooldown restantes no slot (0 = pronto). */
	float GetAbilityCooldownRemaining(int32 Slot) const;

protected:
	/** Servidor: dispara um projétil para frente, com dano, cor e tamanho. */
	void SpawnAttackProjectile(float Damage, FLinearColor Color, float Scale);

	/** Servidor: reage à morte deste personagem. */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

private:
	bool bDeadHandled = false;
	float LastAbilityTime[2] = { -100.f, -100.f };
};
