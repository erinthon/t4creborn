#include "Items/T4CInventoryComponent.h"
#include "GAS/T4CAbilitySystemComponent.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CGameplayTags.h"
#include "GAS/Effects/T4CGameplayEffects.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
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

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		const UT4CAttributeSet* Set = ASC ? ASC->GetSet<UT4CAttributeSet>() : nullptr;
		if (!ASC || !Set || !Set->IsAlive())
		{
			return false; // sem ASC vivo: não desperdiça a poção
		}

		const float Heal = Items[i].HealAmount;
		// Cura via GE_Heal (SetByCaller Data.Healing).
		FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UGE_Heal::StaticClass(), 1.f, Ctx);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(T4CTags::Data_Healing, Heal);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
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
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		if (UT4CAbilitySystemComponent* T4CASC = Cast<UT4CAbilitySystemComponent>(ASC))
		{
			T4CASC->ApplyEquipment(GetEquippedArmor(), GetEquippedWeaponDamage());
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
