#pragma once

#include "NativeGameplayTags.h"

/**
 * Tags nativas do GAS para T4C: Reborn. Declaradas como variáveis nativas
 * (resolvidas em tempo de carga, sem strings espalhadas pelo código).
 *
 * Convenção:
 *  - State.*      estado do ator (morto, defendendo)
 *  - Cooldown.*   tag concedida pelo GE de cooldown de cada slot
 *  - Data.*       chaves de SetByCaller (magnitudes dinâmicas em GEs)
 *  - Ability.*    classificação das habilidades
 */
namespace T4CTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Parrying);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Q);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_E);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Healing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_ManaCost);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Duration);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Projectile);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Heal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Parry);
}
