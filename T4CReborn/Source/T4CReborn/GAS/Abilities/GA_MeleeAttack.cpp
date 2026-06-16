#include "GAS/Abilities/GA_MeleeAttack.h"
#include "GAS/T4CAttributeSet.h"
#include "Core/T4CPlayerState.h"
#include "Player/T4CCharacter.h"
#include "Attributes/T4CAttributeData.h"
#include "AbilitySystemComponent.h"

void UGA_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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
		AT4CCharacter* Char = Cast<AT4CCharacter>(ActorInfo->AvatarActor.Get());
		const AT4CPlayerState* PS = Cast<AT4CPlayerState>(ActorInfo->OwnerActor.Get());
		const UT4CAttributeSet* Set = ActorInfo->AbilitySystemComponent.IsValid()
			? ActorInfo->AbilitySystemComponent->GetSet<UT4CAttributeSet>() : nullptr;

		if (Char && PS && Set)
		{
			// Escala pelo atributo da habilidade (Str p/ golpe, Agi p/ backstab).
			const int32 Scale = PS->GetPrimaryStats().Get(Data.ScaleAttr);
			const FT4CBalanceConstants Balance;
			const float Mul = 1.f + Scale * Balance.DamagePerStrength;
			const float WeaponBase = Char->GetBaseWeaponDamage() + Set->GetWeaponDamageBonus();
			const float Damage = WeaponBase * Mul * Data.Power;

			Char->DoMeleeSweep(220.f, Damage);
			Char->PlayAttackAnim();
			UE_LOG(LogTemp, Verbose, TEXT("[T4C] GA Melee '%s' (dano %.0f, escala %d)"), *Data.Name, Damage, Scale);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
