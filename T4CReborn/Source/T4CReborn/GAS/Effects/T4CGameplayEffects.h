#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "T4CGameplayEffects.generated.h"

/** Infinito: deriva MaxHealth/MaxMana dos atributos primários (via MMCs). */
UCLASS()
class T4CREBORN_API UGE_DerivedAttributes : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_DerivedAttributes();
};

/** Infinito periódico (0.25s): regenera HP e Mana ao longo do tempo. */
UCLASS()
class T4CREBORN_API UGE_Regen : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_Regen();
};

/** Instantâneo: dano via execution (mitiga armadura) → IncomingDamage. */
UCLASS()
class T4CREBORN_API UGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_Damage();
};

/** Instantâneo: cura via SetByCaller(Data.Healing) → IncomingHealing. */
UCLASS()
class T4CREBORN_API UGE_Heal : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_Heal();
};

/** Instantâneo: custo de mana via SetByCaller(Data.ManaCost) (valor negativo). */
UCLASS()
class T4CREBORN_API UGE_Cost : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_Cost();
};

/** Duração (SetByCaller Data.Duration): concede Armadura temporária (Parry). */
UCLASS()
class T4CREBORN_API UGE_Parry : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_Parry();
};

/** Duração (SetByCaller Data.Duration): cooldown puro. A tag de cooldown do slot
 *  é adicionada dinamicamente no ApplyCooldown da habilidade. */
UCLASS()
class T4CREBORN_API UGE_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_Cooldown();
};
