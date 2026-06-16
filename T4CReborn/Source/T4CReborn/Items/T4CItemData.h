#pragma once

#include "CoreMinimal.h"
#include "T4CItemData.generated.h"

/** Categoria de um item. Define qual slot ele ocupa e o efeito que aplica. */
UENUM(BlueprintType)
enum class ET4CItemType : uint8
{
	Weapon  UMETA(DisplayName = "Arma"),    // soma WeaponDamage ao dano de projétil
	Armor   UMETA(DisplayName = "Armadura"),// soma Armor (redução plana de dano)
	Potion  UMETA(DisplayName = "Poção")    // consumível: cura HealAmount
};

/**
 * Um item de Althea (loot). Replicado dentro do inventário, então todos os
 * campos são UPROPERTY. Itens são "achatados" (sem subclasses) para caber numa
 * TArray replicada simples — fiel ao espírito do loot direto do T4C.
 */
USTRUCT(BlueprintType)
struct FT4CItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "T4C|Item") FName Id;
	UPROPERTY(BlueprintReadOnly, Category = "T4C|Item") FString Name;
	UPROPERTY(BlueprintReadOnly, Category = "T4C|Item") ET4CItemType Type = ET4CItemType::Weapon;

	/** Bônus de dano de arma (somado ao dano base das habilidades de projétil). */
	UPROPERTY(BlueprintReadOnly, Category = "T4C|Item") float WeaponDamage = 0.f;

	/** Redução plana de dano recebido (armadura). */
	UPROPERTY(BlueprintReadOnly, Category = "T4C|Item") float Armor = 0.f;

	/** Cura concedida ao usar (poções). */
	UPROPERTY(BlueprintReadOnly, Category = "T4C|Item") float HealAmount = 0.f;

	/** 0 = comum (cinza), 1 = incomum (verde), 2 = raro (dourado). Para cor no mundo/HUD. */
	UPROPERTY(BlueprintReadOnly, Category = "T4C|Item") int32 Rarity = 0;

	FT4CItem() {}
	FT4CItem(FName InId, const FString& InName, ET4CItemType InType,
		float InWeapon, float InArmor, float InHeal, int32 InRarity)
		: Id(InId), Name(InName), Type(InType)
		, WeaponDamage(InWeapon), Armor(InArmor), HealAmount(InHeal), Rarity(InRarity) {}

	bool IsValid() const { return !Id.IsNone(); }

	/** Valor de compra em ouro (derivado de raridade + atributos do item). */
	int32 BuyValue() const
	{
		return FMath::RoundToInt(10.f + Rarity * 25.f + WeaponDamage * 2.f + Armor * 3.f + HealAmount * 0.4f);
	}

	/** Valor de venda em ouro (metade do de compra, mín. 1). */
	int32 SellValue() const { return FMath::Max(1, BuyValue() / 2); }

	/** Cor associada à raridade (mundo e HUD). */
	FLinearColor RarityColor() const
	{
		switch (Rarity)
		{
		case 2:  return FLinearColor(1.0f, 0.85f, 0.2f);  // raro: dourado
		case 1:  return FLinearColor(0.25f, 0.9f, 0.35f); // incomum: verde
		default: return FLinearColor(0.8f, 0.8f, 0.85f);  // comum: cinza-prata
		}
	}
};

/**
 * Tabela de itens dropáveis e a rolagem de loot. Mantida em código (como as
 * classes/habilidades) no protótipo; migrar para DataTable na Fase 4.
 */
namespace T4CItems
{
	/** Loot table do protótipo (índice usado pela rolagem ponderada abaixo). */
	inline const TArray<FT4CItem>& DropTable()
	{
		static const TArray<FT4CItem> Table = {
			FT4CItem(TEXT("potion_minor"),  TEXT("Poção Menor"),       ET4CItemType::Potion, 0.f,  0.f, 40.f,  0),
			FT4CItem(TEXT("dagger_rusty"),  TEXT("Adaga Enferrujada"), ET4CItemType::Weapon, 4.f,  0.f,  0.f,  0),
			FT4CItem(TEXT("armor_leather"), TEXT("Armadura de Couro"), ET4CItemType::Armor,  0.f,  3.f,  0.f,  0),
			FT4CItem(TEXT("sword_iron"),    TEXT("Espada de Ferro"),   ET4CItemType::Weapon, 10.f, 0.f,  0.f,  1),
			FT4CItem(TEXT("armor_chain"),   TEXT("Cota de Malha"),     ET4CItemType::Armor,  0.f,  7.f,  0.f,  1),
			FT4CItem(TEXT("potion_major"),  TEXT("Poção Maior"),       ET4CItemType::Potion, 0.f,  0.f, 90.f,  1),
			FT4CItem(TEXT("sword_great"),   TEXT("Montante de Aço"),   ET4CItemType::Weapon, 18.f, 0.f,  0.f,  2),
			FT4CItem(TEXT("armor_plate"),   TEXT("Armadura de Placas"),ET4CItemType::Armor,  0.f, 12.f,  0.f,  2),
		};
		return Table;
	}

	/**
	 * Rola um item de loot. Raridades mais altas são mais raras.
	 * Pesos: comum 60%, incomum 30%, raro 10%; dentro da raridade, uniforme.
	 */
	inline FT4CItem Roll()
	{
		const float R = FMath::FRand();
		int32 TargetRarity = 0;
		if (R > 0.90f)      TargetRarity = 2;
		else if (R > 0.60f) TargetRarity = 1;

		// Coleta os itens da raridade alvo (cai para raridade menor se vazia).
		TArray<const FT4CItem*> Pool;
		for (int32 Rarity = TargetRarity; Rarity >= 0 && Pool.Num() == 0; --Rarity)
		{
			for (const FT4CItem& It : DropTable())
			{
				if (It.Rarity == Rarity)
				{
					Pool.Add(&It);
				}
			}
		}

		if (Pool.Num() == 0)
		{
			return FT4CItem();
		}
		return *Pool[FMath::RandRange(0, Pool.Num() - 1)];
	}
}
