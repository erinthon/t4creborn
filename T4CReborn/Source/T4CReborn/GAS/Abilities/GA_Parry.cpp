#include "GAS/Abilities/GA_Parry.h"
#include "GAS/T4CGameplayTags.h"
#include "GAS/Effects/T4CGameplayEffects.h"
#include "AbilitySystemComponent.h"

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
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			// GE_Parry usa dois SetByCaller: duração e a fração de redução (Power).
			FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UGE_Parry::StaticClass(), 1.f, Ctx);
			if (Spec.IsValid())
			{
				Spec.Data->SetSetByCallerMagnitude(T4CTags::Data_Duration, Data.Duration);
				Spec.Data->SetSetByCallerMagnitude(T4CTags::Data_Reduction, Data.Power);
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
