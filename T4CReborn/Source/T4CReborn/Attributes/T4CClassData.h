#pragma once

#include "CoreMinimal.h"
#include "Attributes/T4CAttributeData.h"
#include "T4CClassData.generated.h"

/** As 8 classes jogáveis de The 4th Coming (fiel ao T4C Bible). */
UENUM(BlueprintType)
enum class ET4CClass : uint8
{
	Warrior     UMETA(DisplayName = "Guerreiro"),
	Mage        UMETA(DisplayName = "Mago"),
	Paladin     UMETA(DisplayName = "Paladino"),
	BattleMage  UMETA(DisplayName = "Battle Mage"),
	Archer      UMETA(DisplayName = "Arqueiro"),
	Cleric      UMETA(DisplayName = "Clérigo"),
	Rogue       UMETA(DisplayName = "Ladino"),
	Healer      UMETA(DisplayName = "Curandeiro")
};

/** Definição de uma classe: nome e atributos iniciais ("roll"). */
USTRUCT(BlueprintType)
struct FT4CClassDef
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "T4C")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "T4C")
	FT4CPrimaryStats Stats;

	FT4CClassDef() {}
	FT4CClassDef(const FString& InName, int32 Str, int32 End, int32 Agi, int32 Int, int32 Wis)
		: Name(InName)
	{
		Stats.Strength = Str;
		Stats.Endurance = End;
		Stats.Agility = Agi;
		Stats.Intelligence = Int;
		Stats.Wisdom = Wis;
	}
};

/** Tabela das classes. Rolls de referência herdados do T4C Bible (ver GDD §3). */
namespace T4CClasses
{
	static constexpr int32 Count = 8;

	inline FT4CClassDef Get(ET4CClass Class)
	{
		switch (Class)
		{
		case ET4CClass::Warrior:    return FT4CClassDef(TEXT("Guerreiro"),  22, 22, 15,  8,  8);
		case ET4CClass::Mage:       return FT4CClassDef(TEXT("Mago"),        8, 14, 10, 20, 20);
		case ET4CClass::Paladin:    return FT4CClassDef(TEXT("Paladino"),   20, 18, 12,  8, 18);
		case ET4CClass::BattleMage: return FT4CClassDef(TEXT("Battle Mage"),14, 14, 14, 20, 10);
		case ET4CClass::Archer:     return FT4CClassDef(TEXT("Arqueiro"),   18, 20, 20,  8,  8);
		case ET4CClass::Cleric:     return FT4CClassDef(TEXT("Clérigo"),    10, 16, 10, 12, 22);
		case ET4CClass::Rogue:      return FT4CClassDef(TEXT("Ladino"),     12, 14, 20, 14, 14);
		case ET4CClass::Healer:     return FT4CClassDef(TEXT("Curandeiro"), 15, 15, 10, 14, 20);
		default:                    return FT4CClassDef(TEXT("Guerreiro"),  22, 22, 15,  8,  8);
		}
	}

	inline ET4CClass FromIndex(int32 Index)
	{
		return static_cast<ET4CClass>(FMath::Clamp(Index, 0, Count - 1));
	}
}
