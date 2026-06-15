#include "Items/T4CInventoryComponent.h"
#include "Attributes/T4CAttributeComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

UT4CInventoryComponent::UT4CInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UT4CInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UT4CInventoryComponent, Items);
	DOREPLIFETIME(UT4CInventoryComponent, EquippedWeaponIndex);
	DOREPLIFETIME(UT4CInventoryComponent, EquippedArmorIndex);
}

void UT4CInventoryComponent::AddItem(const FT4CItem& Item)
{
	if (GetOwnerRole() != ROLE_Authority || !Item.IsValid())
	{
		return;
	}

	const int32 NewIndex = Items.Add(Item);
	UE_LOG(LogTemp, Display, TEXT("[T4C] Inventario +1: %s (slot %d)"), *Item.Name, NewIndex);

	AutoEquipIfBetter(NewIndex);
	RefreshEquipmentBonuses();
	OnInventoryChanged.Broadcast();
}

void UT4CInventoryComponent::AutoEquipIfBetter(int32 NewIndex)
{
	if (!Items.IsValidIndex(NewIndex))
	{
		return;
	}
	const FT4CItem& Item = Items[NewIndex];

	if (Item.Type == ET4CItemType::Weapon)
	{
		const bool bBetter = !Items.IsValidIndex(EquippedWeaponIndex)
			|| Item.WeaponDamage > Items[EquippedWeaponIndex].WeaponDamage;
		if (bBetter)
		{
			EquippedWeaponIndex = NewIndex;
			UE_LOG(LogTemp, Display, TEXT("[T4C] Equipou arma: %s (+%.0f dano)"), *Item.Name, Item.WeaponDamage);
		}
	}
	else if (Item.Type == ET4CItemType::Armor)
	{
		const bool bBetter = !Items.IsValidIndex(EquippedArmorIndex)
			|| Item.Armor > Items[EquippedArmorIndex].Armor;
		if (bBetter)
		{
			EquippedArmorIndex = NewIndex;
			UE_LOG(LogTemp, Display, TEXT("[T4C] Equipou armadura: %s (+%.0f armadura)"), *Item.Name, Item.Armor);
		}
	}
}

bool UT4CInventoryComponent::UseFirstPotion()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].Type != ET4CItemType::Potion)
		{
			continue;
		}

		UT4CAttributeComponent* Attr = nullptr;
		if (const APlayerState* PS = Cast<APlayerState>(GetOwner()))
		{
			if (const APawn* Pawn = PS->GetPawn())
			{
				Attr = Pawn->FindComponentByClass<UT4CAttributeComponent>();
			}
		}
		if (!Attr || !Attr->IsAlive())
		{
			return false; // sem pawn vivo: não desperdiça a poção
		}

		const float Heal = Items[i].HealAmount;
		Attr->Heal(Heal);
		UE_LOG(LogTemp, Display, TEXT("[T4C] Usou %s (+%.0f HP)"), *Items[i].Name, Heal);

		Items.RemoveAt(i);
		// Remover desloca índices: corrige os equipados que vinham depois.
		if (EquippedWeaponIndex > i) --EquippedWeaponIndex;
		else if (EquippedWeaponIndex == i) EquippedWeaponIndex = -1;
		if (EquippedArmorIndex > i) --EquippedArmorIndex;
		else if (EquippedArmorIndex == i) EquippedArmorIndex = -1;

		RefreshEquipmentBonuses();
		OnInventoryChanged.Broadcast();
		return true;
	}
	return false;
}

void UT4CInventoryComponent::RefreshEquipmentBonuses()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	if (const APlayerState* PS = Cast<APlayerState>(GetOwner()))
	{
		if (const APawn* Pawn = PS->GetPawn())
		{
			if (UT4CAttributeComponent* Attr = Pawn->FindComponentByClass<UT4CAttributeComponent>())
			{
				Attr->SetEquipment(GetEquippedArmor(), GetEquippedWeaponDamage());
			}
		}
	}
}

float UT4CInventoryComponent::GetEquippedWeaponDamage() const
{
	return Items.IsValidIndex(EquippedWeaponIndex) ? Items[EquippedWeaponIndex].WeaponDamage : 0.f;
}

float UT4CInventoryComponent::GetEquippedArmor() const
{
	return Items.IsValidIndex(EquippedArmorIndex) ? Items[EquippedArmorIndex].Armor : 0.f;
}

FString UT4CInventoryComponent::GetEquippedWeaponName() const
{
	return Items.IsValidIndex(EquippedWeaponIndex) ? Items[EquippedWeaponIndex].Name : FString(TEXT("Punhos"));
}

FString UT4CInventoryComponent::GetEquippedArmorName() const
{
	return Items.IsValidIndex(EquippedArmorIndex) ? Items[EquippedArmorIndex].Name : FString(TEXT("Nenhuma"));
}

void UT4CInventoryComponent::OnRep_Inventory()
{
	OnInventoryChanged.Broadcast();
}
