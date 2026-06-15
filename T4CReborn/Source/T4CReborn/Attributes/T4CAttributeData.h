#pragma once

#include "CoreMinimal.h"
#include "T4CAttributeData.generated.h"

/**
 * Os cinco atributos centrais de The 4th Coming.
 * Fiel ao original: classe é emergente da alocação destes pontos.
 */
UENUM(BlueprintType)
enum class ET4CAttribute : uint8
{
	Strength      UMETA(DisplayName = "Força"),
	Endurance     UMETA(DisplayName = "Vigor"),
	Agility       UMETA(DisplayName = "Agilidade"),
	Intelligence  UMETA(DisplayName = "Inteligência"),
	Wisdom        UMETA(DisplayName = "Sabedoria")
};

/** Bloco dos 5 atributos primários de um personagem. */
USTRUCT(BlueprintType)
struct FT4CPrimaryStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Stats")
	int32 Strength = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Stats")
	int32 Endurance = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Stats")
	int32 Agility = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Stats")
	int32 Intelligence = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Stats")
	int32 Wisdom = 10;

	int32 Get(ET4CAttribute Attr) const
	{
		switch (Attr)
		{
		case ET4CAttribute::Strength:     return Strength;
		case ET4CAttribute::Endurance:    return Endurance;
		case ET4CAttribute::Agility:      return Agility;
		case ET4CAttribute::Intelligence: return Intelligence;
		case ET4CAttribute::Wisdom:       return Wisdom;
		default:                          return 0;
		}
	}

	void Add(ET4CAttribute Attr, int32 Amount)
	{
		switch (Attr)
		{
		case ET4CAttribute::Strength:     Strength     += Amount; break;
		case ET4CAttribute::Endurance:    Endurance    += Amount; break;
		case ET4CAttribute::Agility:      Agility       += Amount; break;
		case ET4CAttribute::Intelligence: Intelligence += Amount; break;
		case ET4CAttribute::Wisdom:       Wisdom        += Amount; break;
		default: break;
		}
	}
};

/**
 * Constantes de balanceamento (k_* do GDD). Exposto como linha de DataTable
 * para tuning sem recompilar. Ver Content/Data/DT_GameBalance.
 */
USTRUCT(BlueprintType)
struct FT4CBalanceConstants
{
	GENERATED_BODY()

	// HP base + END * HPPerEndurance concedido no level-up.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	float BaseHP = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	float HPPerEndurance = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	float BaseMana = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	float ManaPerIntelligence = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	float ManaPerWisdom = 2.f;

	// DanoMelee = ArmaBase * (1 + STR * DamagePerStrength)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	float DamagePerStrength = 0.05f;

	// Regeneração por segundo (mana mais rápida que vida).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	float ManaRegenPerSec = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	float HealthRegenPerSec = 2.f;

	// Pontos concedidos por nível (fiel ao T4C: 5 atributo / 15 perícia).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	int32 StatPointsPerLevel = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T4C|Balance")
	int32 SkillPointsPerLevel = 15;
};
