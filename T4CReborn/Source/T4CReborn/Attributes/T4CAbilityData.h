#pragma once

#include "CoreMinimal.h"
#include "Attributes/T4CClassData.h"
#include "Attributes/T4CAttributeData.h"
#include "T4CAbilityData.generated.h"

/** Tipo de efeito de uma habilidade. */
UENUM(BlueprintType)
enum class ET4CAbilityKind : uint8
{
	Projectile,  // dispara um projétil (mágico/à distância); escala pelo atributo da habilidade
	Melee,       // golpe de proximidade (sweep curto à frente)
	Heal,        // cura o conjurador (Power + escala WIS)
	Parry        // redução de dano temporária (Power = fração, Duration = s); também "Hide"
};

/** Definição de uma habilidade (perícia/magia), fiel às skills do T4C Bible. */
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
	/** Atributo que escala o efeito (Str p/ golpe, Agi p/ tiro/backstab, Int/Wis p/ magia). */
	UPROPERTY(BlueprintReadOnly, Category = "T4C") ET4CAttribute ScaleAttr = ET4CAttribute::Strength;

	FT4CAbility() {}
	FT4CAbility(const FString& InName, ET4CAbilityKind InKind, float InCooldown, float InMana,
		float InPower, float InDuration = 0.f, ET4CAttribute InScale = ET4CAttribute::Strength)
		: Name(InName), Kind(InKind), Cooldown(InCooldown), ManaCost(InMana)
		, Power(InPower), Duration(InDuration), ScaleAttr(InScale) {}
};

/**
 * Loadout de habilidades por classe (Slot 0 = Q, Slot 1 = E), fiel ao T4C Bible
 * (t4cbible.com/charclass + /skills). Melee não custa mana; magias custam.
 */
namespace T4CAbilities
{
	inline FT4CAbility Get(ET4CClass Class, int32 Slot)
	{
		using K = ET4CAbilityKind;
		using A = ET4CAttribute;
		switch (Class)
		{
		case ET4CClass::Warrior: // melee puro + defesa
			return Slot == 0 ? FT4CAbility(TEXT("Powerful Blow"), K::Melee, 2.0f, 0.f, 2.5f, 0.f, A::Strength)
			                 : FT4CAbility(TEXT("Parry"), K::Parry, 8.f, 0.f, 0.5f, 4.f);
		case ET4CClass::Mage: // linha de Fogo
			return Slot == 0 ? FT4CAbility(TEXT("Fire Dart"), K::Projectile, 1.5f, 10.f, 2.0f, 0.f, A::Intelligence)
			                 : FT4CAbility(TEXT("FireStorm"), K::Projectile, 6.f, 25.f, 3.5f, 0.f, A::Intelligence);
		case ET4CClass::Paladin: // melee + suporte
			return Slot == 0 ? FT4CAbility(TEXT("Powerful Blow"), K::Melee, 2.0f, 0.f, 2.0f, 0.f, A::Strength)
			                 : FT4CAbility(TEXT("Heal"), K::Heal, 8.f, 15.f, 30.f, 0.f, A::Wisdom);
		case ET4CClass::BattleMage: // melee + fogo
			return Slot == 0 ? FT4CAbility(TEXT("Powerful Blow"), K::Melee, 2.0f, 0.f, 2.0f, 0.f, A::Strength)
			                 : FT4CAbility(TEXT("Fire Dart"), K::Projectile, 1.5f, 10.f, 2.0f, 0.f, A::Intelligence);
		case ET4CClass::Archer: // ranged + esquiva
			return Slot == 0 ? FT4CAbility(TEXT("Power Shot"), K::Projectile, 2.0f, 0.f, 2.6f, 0.f, A::Agility)
			                 : FT4CAbility(TEXT("Parry"), K::Parry, 8.f, 0.f, 0.4f, 3.f);
		case ET4CClass::Cleric: // smite + cura
			return Slot == 0 ? FT4CAbility(TEXT("Smite"), K::Projectile, 2.f, 8.f, 2.2f, 0.f, A::Wisdom)
			                 : FT4CAbility(TEXT("Heal"), K::Heal, 6.f, 12.f, 25.f, 0.f, A::Wisdom);
		case ET4CClass::Rogue: // backstab + stealth (Hide = evasão)
			return Slot == 0 ? FT4CAbility(TEXT("Backstab"), K::Melee, 2.5f, 0.f, 3.0f, 0.f, A::Agility)
			                 : FT4CAbility(TEXT("Hide"), K::Parry, 7.f, 0.f, 0.5f, 3.f);
		case ET4CClass::Healer: // terra + cura forte
			return Slot == 0 ? FT4CAbility(TEXT("Earth Bolt"), K::Projectile, 2.f, 8.f, 1.8f, 0.f, A::Wisdom)
			                 : FT4CAbility(TEXT("Greater Heal"), K::Heal, 8.f, 25.f, 70.f, 0.f, A::Wisdom);
		default:
			return FT4CAbility(TEXT("Powerful Blow"), K::Melee, 2.0f, 0.f, 2.5f, 0.f, A::Strength);
		}
	}
}
