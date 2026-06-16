#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "Attributes/T4CAttributeData.h"
#include "Attributes/T4CClassData.h"
#include "T4CPlayerState.generated.h"

class UT4CInventoryComponent;
class UT4CAbilitySystemComponent;
class UT4CAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatsChanged);

/**
 * Dados persistentes do personagem: os 5 atributos, nível, XP e pontos
 * não-gastos. Replicado para todos. Mutações ocorrem SOMENTE no servidor.
 */
UCLASS()
class T4CREBORN_API AT4CPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AT4CPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- GAS ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UT4CAbilitySystemComponent* GetT4CAbilitySystemComponent() const { return AbilitySystem; }
	UT4CAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** Servidor: inicializa os atributos do ASC (base + GEs de startup + refill).
	 *  Chamado pelo Character no PossessedBy (também serve para refill no respawn). */
	void InitializeAttributes();

	// --- Persistência (USaveGame em disco) ---
	/** Servidor: grava o personagem no slot (nome do jogador). Só se tem classe. */
	void SaveCharacter();

	/** Servidor: carrega o slot do jogador uma única vez (no 1º spawn). Aplica
	 *  classe/atributos/progressão/inventário e empurra ao ASC. */
	void LoadCharacterOnce();

	/** Nome do slot de save, derivado (sanitizado) do nome do jogador. */
	FString GetSaveSlotName() const;

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	const FT4CPrimaryStats& GetPrimaryStats() const { return PrimaryStats; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	int32 GetCharacterLevel() const { return CharacterLevel; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	int32 GetUnspentStatPoints() const { return UnspentStatPoints; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	int32 GetUnspentSkillPoints() const { return UnspentSkillPoints; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	int32 GetExperience() const { return Experience; }

	/** Servidor: concede XP e processa level-ups. */
	void GrantExperience(int32 Amount);

	/** Cliente → servidor: gasta 1 ponto não-alocado num atributo. */
	UFUNCTION(Server, Reliable)
	void ServerAllocateStat(ET4CAttribute Attribute);

	/** Cliente → servidor: escolhe a classe inicial (define os atributos do roll). */
	UFUNCTION(Server, Reliable)
	void ServerSelectClass(ET4CClass Class);

	/** Cliente → servidor: recomeça o personagem (volta ao menu de classes). */
	UFUNCTION(Server, Reliable)
	void ServerResetClass();

	UFUNCTION(BlueprintPure, Category = "T4C|Inventory")
	UT4CInventoryComponent* GetInventory() const { return Inventory; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	bool HasChosenClass() const { return bHasChosenClass; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	ET4CClass GetChosenClass() const { return ChosenClass; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	FString GetClassName() const { return T4CClasses::Get(ChosenClass).Name; }

	/** XP necessário para alcançar o próximo nível. */
	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	int32 GetXPForNextLevel() const;

	UPROPERTY(BlueprintAssignable, Category = "T4C|Progression")
	FOnStatsChanged OnStatsChanged;

protected:
	/** ASC do jogador. Vive no PlayerState para persistir entre respawns. */
	UPROPERTY(VisibleAnywhere, Category = "T4C|GAS")
	TObjectPtr<UT4CAbilitySystemComponent> AbilitySystem;

	UPROPERTY(VisibleAnywhere, Category = "T4C|GAS")
	TObjectPtr<UT4CAttributeSet> AttributeSet;

	/** Inventário persistente (sobrevive aos respawns do pawn). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T4C|Inventory")
	TObjectPtr<UT4CInventoryComponent> Inventory;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	FT4CPrimaryStats PrimaryStats;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	int32 CharacterLevel = 1;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	int32 Experience = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	int32 UnspentStatPoints = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	int32 UnspentSkillPoints = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	ET4CClass ChosenClass = ET4CClass::Warrior;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	bool bHasChosenClass = false;

	UFUNCTION()
	void OnRep_Progression();

	/** Servidor: avança um nível e concede pontos (fiel ao T4C: 5 stat / 15 skill). */
	void LevelUp();

	/** Servidor: empurra os atributos primários ao ASC; opcionalmente enche vitais. */
	void PushStatsToASC(bool bRefill);

	/** Servidor: concede ao ASC as 2 habilidades (Q/E) da classe escolhida. */
	void GrantClassAbilities();

	/** Servidor: remove as habilidades concedidas (ao recomeçar). */
	void ClearAbilities();

	/** Handles das habilidades concedidas (server-only, não replicado). */
	TArray<FGameplayAbilitySpecHandle> GrantedAbilities;

	// Pontos concedidos por nível — espelham FT4CBalanceConstants.
	static constexpr int32 StatPointsPerLevel = 5;
	static constexpr int32 SkillPointsPerLevel = 15;

private:
	/** Garante que o load do disco ocorre uma única vez por sessão. */
	bool bSaveLoaded = false;
};
