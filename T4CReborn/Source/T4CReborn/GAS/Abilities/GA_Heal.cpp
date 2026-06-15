#include "GAS/Abilities/GA_Heal.h"
#include "GAS/T4CGameplayTags.h"
#include "GAS/Effects/T4CGameplayEffects.h"
#include "Core/T4CPlayerState.h"

void UGA_Heal::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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
		const AT4CPlayerState* PS = Cast<AT4CPlayerState>(ActorInfo->OwnerActor.Get());
		const float Wisdom = PS ? PS->GetPrimaryStats().Wisdom : 0.f;
		ApplySelfEffectSetByCaller(ActorInfo, UGE_Heal::StaticClass(), T4CTags::Data_Healing, Data.Power + Wisdom * 1.5f);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
