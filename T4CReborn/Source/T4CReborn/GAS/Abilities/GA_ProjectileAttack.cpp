#include "GAS/Abilities/GA_ProjectileAttack.h"
#include "GAS/T4CAttributeSet.h"
#include "Core/T4CPlayerState.h"
#include "Player/T4CCharacter.h"
#include "Attributes/T4CAttributeData.h"
#include "AbilitySystemComponent.h"

// Cor do projétil por elemento/perícia (espelha o mapeamento visual antigo).
static FLinearColor ColorForAbility(const FString& Name)
{
	if (Name.Contains(TEXT("Fire")))       return FLinearColor(1.0f, 0.35f, 0.05f);
	if (Name.Contains(TEXT("Smite")))      return FLinearColor(1.0f, 0.85f, 0.25f);
	if (Name.Contains(TEXT("Earth")))      return FLinearColor(0.5f, 0.35f, 0.15f);
	if (Name.Contains(TEXT("Power Shot"))) return FLinearColor(0.3f, 1.0f, 0.35f);
	return FLinearColor(0.9f, 0.9f, 1.0f); // físico: branco-aço
}

void UGA_ProjectileAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicate=*/true, /*bWasCancelled=*/true);
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
			// Escala pelo atributo da habilidade (Int p/ fogo, Wis p/ smite/earth, Agi p/ tiro).
			const int32 Offense = PS->GetPrimaryStats().Get(Data.ScaleAttr);
			const FT4CBalanceConstants Balance;
			const float OffenseMul = 1.f + Offense * Balance.DamagePerStrength;
			const float WeaponBase = Char->GetBaseWeaponDamage() + Set->GetWeaponDamageBonus();
			const float Scale = 0.35f + Data.Power * 0.12f;
			const float FinalDamage = WeaponBase * OffenseMul * Data.Power;

			Char->FireAbilityProjectile(FinalDamage, ColorForAbility(Data.Name), Scale);
			Char->PlayAttackAnim();
			UE_LOG(LogTemp, Verbose, TEXT("[T4C] GA Projectile '%s' disparada (dano %.0f)"), *Data.Name, FinalDamage);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicate=*/true, /*bWasCancelled=*/false);
}
