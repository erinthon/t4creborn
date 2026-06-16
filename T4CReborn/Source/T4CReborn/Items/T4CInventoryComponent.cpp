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

bool UT4CInventoryComponent::UseFirstHealthPotion() { return UsePotionInternal(ET4CItemType::Potion, /*bMana=*/false); }
bool UT4CInventoryComponent::UseFirstManaPotion()   { return UsePotionInternal(ET4CItemType::ManaPotion, /*bMana=*/true); }

bool UT4CInventoryComponent::UsePotionInternal(ET4CItemType PotionType, bool bMana)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].Type != PotionType)
		{
			continue;
		}

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		const UT4CAttributeSet* Set = ASC ? ASC->GetSet<UT4CAttributeSet>() : nullptr;
		if (!ASC || !Set || !Set->IsAlive())
		{
			return false; // sem ASC vivo: não desperdiça a poção
		}

		const float Amount = Items[i].HealAmount;
		// Vida via GE_Heal; Mana via GE_Cost (que soma à Mana pelo SetByCaller, valor positivo).
		const TSubclassOf<UGameplayEffect> EffectClass = bMana ? UGE_Cost::StaticClass() : UGE_Heal::StaticClass();
		const FGameplayTag DataTag = bMana ? T4CTags::Data_ManaCost : T4CTags::Data_Healing;
		FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectClass, 1.f, Ctx);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(DataTag, Amount);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
		UE_LOG(LogTemp, Display, TEXT("[T4C] Usou %s (+%.0f %s)"),
			*Items[i].Name, Amount, bMana ? TEXT("Mana") : TEXT("HP"));

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

int32 UT4CInventoryComponent::CountItemsOfType(ET4CItemType Type) const
{
	int32 N = 0;
	for (const FT4CItem& Item : Items) { if (Item.Type == Type) ++N; }
	return N;
}

void UT4CInventoryComponent::RestoreFromSave(const TArray<FT4CItem>& InItems, int32 WeaponIdx, int32 ArmorIdx)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	Items = InItems;
	EquippedWeaponIndex = Items.IsValidIndex(WeaponIdx) ? WeaponIdx : -1;
	EquippedArmorIndex = Items.IsValidIndex(ArmorIdx) ? ArmorIdx : -1;
	RefreshEquipmentBonuses();
	OnInventoryChanged.Broadcast();
}

int32 UT4CInventoryComponent::SellAllUnequipped()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return 0;
	}

	int32 Total = 0;
	TArray<FT4CItem> Kept;
	int32 NewWeapon = -1, NewArmor = -1;
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (i == EquippedWeaponIndex || i == EquippedArmorIndex)
		{
			const int32 NewIdx = Kept.Add(Items[i]);
			if (i == EquippedWeaponIndex) NewWeapon = NewIdx;
			if (i == EquippedArmorIndex)  NewArmor = NewIdx;
		}
		else
		{
			Total += Items[i].SellValue();
		}
	}

	if (Kept.Num() == Items.Num())
	{
		return 0; // nada para vender (tudo equipado ou vazio)
	}

	Items = Kept;
	EquippedWeaponIndex = NewWeapon;
	EquippedArmorIndex = NewArmor;
	RefreshEquipmentBonuses();
	OnInventoryChanged.Broadcast();
	return Total;
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
