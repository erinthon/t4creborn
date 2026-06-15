#include "GAS/Abilities/GA_Parry.h"
#include "GAS/T4CGameplayTags.h"
#include "GAS/Effects/T4CGameplayEffects.h"

void UGA_Parry::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ActorInfo->IsNetAuthority())
	{
		const FT4CAbility Data = GetAbilityData(ActorInfo, Handle);
		ApplySelfEffectSetByCaller(ActorInfo, UGE_Parry::StaticClass(), T4CTags::Data_Duration, Data.Duration);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
