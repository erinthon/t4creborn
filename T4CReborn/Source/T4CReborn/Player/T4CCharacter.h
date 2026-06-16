#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GAS/T4CCombatant.h"
#include "Attributes/T4CAttributeData.h"
#include "T4CCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UT4CAttributeSet;
class UAnimSequenceBase;

/**
 * Pawn jogável de Althea. Câmera em terceira pessoa (action-RPG).
 * Autossuficiente: corpo visível e bindings de input definidos em código/config,
 * sem necessidade de Blueprint. Combate e atributos são autoritativos no servidor.
 *
 * O ASC vive no PlayerState (persiste entre respawns); o Character é o avatar.
 */
UCLASS()
class T4CREBORN_API AT4CCharacter : public ACharacter, public IAbilitySystemInterface, public IT4CCombatant
{
	GENERATED_BODY()

public:
	AT4CCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// --- GAS ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UT4CAttributeSet* GetAttributeSet() const;

	UFUNCTION(BlueprintPure, Category = "T4C")
	bool IsAlive() const;

	/** IT4CCombatant: servidor reage à morte (XP ao matador, respawn). */
	virtual void HandleDeath(AActor* Killer) override;

protected:
	virtual void BeginPlay() override;

	/** Servidor e cliente: liga o ASC do PlayerState a este avatar. */
	void InitAbilitySystem();

	// O corpo visível é a malha esqueletal herdada de ACharacter (GetMesh()):
	// Manny (SKM_Manny_Simple) + AnimBlueprint ABP_Unarmed, montados no construtor.

	/** Arma na mão direita (placeholder de primitiva; trocar por malha real depois). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

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

	// Habilidades de classe via GAS: a tecla ativa pelo InputID no ASC; a
	// GameplayAbility concedida pela classe trata custo/cooldown/efeito.
	void OnAbilityQPressed();
	void OnAbilityEPressed();

	// --- Loot / inventário / NPC (RPCs; handlers de input são públicos abaixo) ---
	UFUNCTION(Server, Reliable)
	void ServerInteract();

	UFUNCTION(Server, Reliable)
	void ServerUsePotion();

	UFUNCTION(Server, Reliable)
	void ServerUseManaPotion();

	UFUNCTION(Server, Reliable)
	void ServerBuy();

	/** Toca a animação de ataque em todos os clientes. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttack(int32 Index);

	/** Animações de ataque (sequences) embrulhadas em montage dinâmica ao usar. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<TObjectPtr<UAnimSequenceBase>> AttackAnims;

public:
	/** Tecla F: coleta loot próximo ou interage com o NPC mais próximo. */
	void Interact();
	/** Tecla G: usa a primeira poção de vida. */
	void UsePotion();
	/** Tecla H: usa a primeira poção de mana. */
	void UseManaPotion();
	/** Tecla B: compra uma poção do mercador próximo. */
	void Buy();

	/** Para o HUD: nome da habilidade no slot (0=Q, 1=E). */
	FString GetAbilityName(int32 Slot) const;

	/** Para o HUD: segundos de cooldown restantes no slot (0 = pronto). Lê o GE
	 *  de cooldown no ASC (replicado ao dono). */
	float GetAbilityCooldownRemaining(int32 Slot) const;

	/** Dano base das habilidades de projétil (lido pela GameplayAbility). */
	float GetBaseWeaponDamage() const { return BaseWeaponDamage; }

	/** Servidor: dispara o projétil de ataque (chamado pela GameplayAbility). */
	void FireAbilityProjectile(float Damage, FLinearColor Color, float Scale) { SpawnAttackProjectile(Damage, Color, Scale); }

	/** Servidor: golpe melee — sweep de esfera curto à frente, aplica dano via ASC. */
	void DoMeleeSweep(float Range, float Damage);

	/** Servidor: toca uma animação de ataque (replicada a todos via multicast). */
	void PlayAttackAnim();

protected:
	/** Servidor: dispara um projétil para frente, com dano, cor e tamanho. */
	void SpawnAttackProjectile(float Damage, FLinearColor Color, float Scale);

private:
	bool bDeadHandled = false;
	int32 AttackComboIndex = 0; // alterna as animações de ataque (combo)
};
