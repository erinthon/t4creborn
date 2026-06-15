#include "GAS/T4CGameplayTags.h"

namespace T4CTags
{
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Parrying, "State.Parrying");

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Q, "Cooldown.Q");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_E, "Cooldown.E");

	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Healing, "Data.Healing");
	UE_DEFINE_GAMEPLAY_TAG(Data_ManaCost, "Data.ManaCost");
	UE_DEFINE_GAMEPLAY_TAG(Data_Duration, "Data.Duration");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Projectile, "Ability.Type.Projectile");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Heal, "Ability.Type.Heal");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Parry, "Ability.Type.Parry");
}
