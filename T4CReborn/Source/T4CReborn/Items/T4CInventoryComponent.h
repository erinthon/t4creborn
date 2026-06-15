#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/T4CItemData.h"
#include "T4CInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 * Inventário do personagem. Vive no AT4CPlayerState (persiste entre respawns,
 * ao contrário do pawn). Autoridade: SOMENTE o servidor muta; clientes recebem
 * via replicação. Equipa automaticamente arma/armadura melhores ao coletar.
 */
UCLASS(ClassGroup = (T4C), meta = (BlueprintSpawnableComponent))
class T4CREBORN_API UT4CInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT4CInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Servidor: adiciona um item ao inventário e auto-equipa se for melhor. */
	void AddItem(const FT4CItem& Item);

	/** Servidor: usa a primeira poção do inventário (cura o pawn). Retorna true se usou. */
	bool UseFirstPotion();

	/** Servidor: reaplica os bônus de equipamento aos atributos do ASC
	 *  (chamar após coletar/respawnar). */
	void RefreshEquipmentBonuses();

	// --- Consultas (HUD) ---
	UFUNCTION(BlueprintPure, Category = "T4C|Inventory")
	const TArray<FT4CItem>& GetItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "T4C|Inventory")
	float GetEquippedWeaponDamage() const;

	UFUNCTION(BlueprintPure, Category = "T4C|Inventory")
	float GetEquippedArmor() const;

	UFUNCTION(BlueprintPure, Category = "T4C|Inventory")
	FString GetEquippedWeaponName() const;

	UFUNCTION(BlueprintPure, Category = "T4C|Inventory")
	FString GetEquippedArmorName() const;

	/** Índice no array Items do item equipado em cada slot (-1 = nenhum). */
	int32 GetEquippedWeaponIndex() const { return EquippedWeaponIndex; }
	int32 GetEquippedArmorIndex() const { return EquippedArmorIndex; }

	UPROPERTY(BlueprintAssignable, Category = "T4C|Inventory")
	FOnInventoryChanged OnInventoryChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Inventory, VisibleAnywhere, Category = "T4C|Inventory")
	TArray<FT4CItem> Items;

	UPROPERTY(ReplicatedUsing = OnRep_Inventory, VisibleAnywhere, Category = "T4C|Inventory")
	int32 EquippedWeaponIndex = -1;

	UPROPERTY(ReplicatedUsing = OnRep_Inventory, VisibleAnywhere, Category = "T4C|Inventory")
	int32 EquippedArmorIndex = -1;

	UFUNCTION()
	void OnRep_Inventory();

private:
	/** Servidor: equipa o item do índice se melhorar o slot correspondente. */
	void AutoEquipIfBetter(int32 NewIndex);
};
