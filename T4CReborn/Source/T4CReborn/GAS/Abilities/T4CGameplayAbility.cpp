#include "GAS/Abilities/T4CGameplayAbility.h"
#include "GAS/T4CAbilityInputID.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CGameplayTags.h"
#include "GAS/Effects/T4CGameplayEffects.h"
#include "Core/T4CPlayerState.h"
#include "AbilitySystemComponent.h"

UT4CGameplayAbility::UT4CGameplayAbility()
{
	// Instância por ator (mantém estado por dono) e execução autoritativa no
	// servidor — o input do cliente é roteado via ASC. Predição fica para depois.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	CostEffectClass = UGE_Cost::StaticClass();
	CooldownEffectClass = UGE_Cooldown::StaticClass();
}

int32 UT4CGameplayAbility::GetSlot(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle) const
{
	if (ActorInfo)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			if (const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle))
			{
				return (Spec->InputID == static_cast<int32>(ET4CAbilityInputID::AbilityE)) ? 1 : 0;
			}
		}
	}
	return 0;
}

FGameplayTag UT4CGameplayAbility::GetSlotCooldownTag(int32 Slot) const
{
	return (Slot == 1) ? T4CTags::Cooldown_E : T4CTags::Cooldown_Q;
}

FT4CAbility UT4CGameplayAbility::GetAbilityData(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle) const
{
	const int32 Slot = GetSlot(ActorInfo, Handle);
	if (ActorInfo)
	{
		if (const AT4CPlayerState* PS = Cast<AT4CPlayerState>(ActorInfo->OwnerActor.Get()))
		{
			return T4CAbilities::Get(PS->GetChosenClass(), Slot);
		}
	}
	return FT4CAbility();
}

void UT4CGameplayAbility::ApplySelfEffectSetByCaller(const FGameplayAbilityActorInfo* ActorInfo,
	TSubclassOf<UGameplayEffect> EffectClass, FGameplayTag DataTag, float Magnitude) const
{
	if (!ActorInfo || !EffectClass)
	{
		return;
	}
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}
	FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectClass, 1.f, Ctx);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(DataTag, Magnitude);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}

bool UT4CGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const FT4CAbility Data = GetAbilityData(ActorInfo, Handle);
	if (Data.ManaCost <= 0.f)
	{
		return true;
	}
	if (UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
	{
		if (const UT4CAttributeSet* Set = ASC->GetSet<UT4CAttributeSet>())
		{
			return Set->GetMana() >= Data.ManaCost;
		}
	}
	return false;
}

void UT4CGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const FT4CAbility Data = GetAbilityData(ActorInfo, Handle);
	if (Data.ManaCost > 0.f)
	{
		// GE_Cost soma à Mana, então passamos o custo negativo.
		ApplySelfEffectSetByCaller(ActorInfo, CostEffectClass, T4CTags::Data_ManaCost, -Data.ManaCost);
	}
}

bool UT4CGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const FGameplayTag CDTag = GetSlotCooldownTag(GetSlot(ActorInfo, Handle));
	if (UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
	{
		if (ASC->HasMatchingGameplayTag(CDTag))
		{
			if (OptionalRelevantTags)
			{
				OptionalRelevantTags->AddTag(CDTag);
			}
			return false;
		}
	}
	return true;
}

void UT4CGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const FT4CAbility Data = GetAbilityData(ActorInfo, Handle);
	if (Data.Cooldown <= 0.f || !CooldownEffectClass)
	{
		return;
	}
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC)
	{
		return;
	}
	FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CooldownEffectClass, 1.f, Ctx);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(T4CTags::Data_Duration, Data.Cooldown);
		Spec.Data->DynamicGrantedTags.AddTag(GetSlotCooldownTag(GetSlot(ActorInfo, Handle)));
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}
