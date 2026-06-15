#pragma once

#include "CoreMinimal.h"
#include "Attributes/T4CClassData.h"
#include "T4CAbilityData.generated.h"

/** Tipo de efeito de uma habilidade. */
UENUM(BlueprintType)
enum class ET4CAbilityKind : uint8
{
	Projectile,  // dispara um projétil potente (Power * dano base)
	Heal,        // cura o conjurador (Power + WIS)
	Parry        // redução de dano temporária (Power = fração, Duration = s)
};

/** Definição de uma habilidade (perícia/magia). */
USTRUCT(BlueprintType)
struct FT4CAbility
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "T4C") FString Name;
	UPROPERTY(BlueprintReadOnly, Category = "T4C") ET4CAbilityKind Kind = ET4CAbilityKind::Projectile;
	UPROPERTY(BlueprintReadOnly, Category = "T4C") float Cooldown = 3.f;
	UPROPERTY(BlueprintReadOnly, Category = "T4C") float ManaCost = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "T4C") float Power = 2.f;
	UPROPERTY(BlueprintReadOnly, Category = "T4C") float Duration = 0.f;

	FT4CAbility() {}
	FT4CAbility(const FString& InName, ET4CAbilityKind InKind, float InCooldown, float InMana, float InPower, float InDuration = 0.f)
		: Name(InName), Kind(InKind), Cooldown(InCooldown), ManaCost(InMana), Power(InPower), Duration(InDuration) {}
};

/** Loadout de habilidades por classe (Slot 0 = Q, Slot 1 = E). */
namespace T4CAbilities
{
	inline FT4CAbility Get(ET4CClass Class, int32 Slot)
	{
		using K = ET4CAbilityKind;
		switch (Class)
		{
		case ET4CClass::Warrior:
			return Slot == 0 ? FT4CAbility(TEXT("Powerful Blow"), K::Projectile, 3.f, 0.f, 2.5f)
			                 : FT4CAbility(TEXT("Parry"), K::Parry, 8.f, 0.f, 0.5f, 4.f);
		case ET4CClass::Mage:
			return Slot == 0 ? FT4CAbility(TEXT("Fire Dart"), K::Projectile, 1.5f, 10.f, 2.0f)
			                 : FT4CAbility(TEXT("FireStorm"), K::Projectile, 6.f, 25.f, 3.5f);
		case ET4CClass::Paladin:
			return Slot == 0 ? FT4CAbility(TEXT("Powerful Blow"), K::Projectile, 3.f, 0.f, 2.0f)
			                 : FT4CAbility(TEXT("Heal"), K::Heal, 8.f, 15.f, 30.f);
		case ET4CClass::BattleMage:
			return Slot == 0 ? FT4CAbility(TEXT("Fire Dart"), K::Projectile, 1.5f, 10.f, 2.0f)
			                 : FT4CAbility(TEXT("Powerful Blow"), K::Projectile, 3.f, 0.f, 2.0f);
		case ET4CClass::Archer:
			return Slot == 0 ? FT4CAbility(TEXT("Power Shot"), K::Projectile, 2.5f, 0.f, 2.8f)
			                 : FT4CAbility(TEXT("Parry"), K::Parry, 8.f, 0.f, 0.4f, 3.f);
		case ET4CClass::Cleric:
			return Slot == 0 ? FT4CAbility(TEXT("Heal"), K::Heal, 5.f, 12.f, 25.f)
			                 : FT4CAbility(TEXT("Parry"), K::Parry, 8.f, 0.f, 0.5f, 4.f);
		case ET4CClass::Rogue:
			return Slot == 0 ? FT4CAbility(TEXT("Backstab"), K::Projectile, 2.5f, 0.f, 3.0f)
			                 : FT4CAbility(TEXT("Parry"), K::Parry, 7.f, 0.f, 0.4f, 3.f);
		case ET4CClass::Healer:
			return Slot == 0 ? FT4CAbility(TEXT("Heal"), K::Heal, 4.f, 10.f, 30.f)
			                 : FT4CAbility(TEXT("Greater Heal"), K::Heal, 10.f, 25.f, 70.f);
		default:
			return FT4CAbility(TEXT("Powerful Blow"), K::Projectile, 3.f, 0.f, 2.5f);
		}
	}
}
