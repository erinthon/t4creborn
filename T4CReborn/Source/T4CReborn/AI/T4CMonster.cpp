#include "AI/T4CMonster.h"
#include "AI/T4CMonsterAIController.h"
#include "Player/T4CCharacter.h"
#include "Core/T4CPlayerState.h"
#include "Core/T4CGameMode.h"
#include "GAS/T4CAbilitySystemComponent.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CGameplayTags.h"
#include "GAS/Effects/T4CGameplayEffects.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AT4CMonster::AT4CMonster()
{
	PrimaryActorTick.bCanEverTick = false;

	// Stats modestos: END 6 => ~54 HP com o balance padrao.
	Stats.Strength = 12;
	Stats.Endurance = 6;
	Stats.Agility = 8;
	Stats.Intelligence = 4;
	Stats.Wisdom = 4;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 320.f; // mais lento que o jogador

	// Corpo: cubo do engine (distinto dos jogadores, que sao cilindros).
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CubeMesh.Object);
		BodyMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.7f));
		BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -10.f));
	}

	// ASC próprio (monstros não têm PlayerState). Replicação mínima: os outros
	// clientes só precisam dos vitais para as barras de vida do HUD.
	AbilitySystem = CreateDefaultSubobject<UT4CAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystem->SetIsReplicated(true);
	AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UT4CAttributeSet>(TEXT("AttributeSet"));

	AIControllerClass = AT4CMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UAbilitySystemComponent* AT4CMonster::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

bool AT4CMonster::IsAlive() const
{
	return AttributeSet && AttributeSet->IsAlive();
}

void AT4CMonster::InitAbilitySystem()
{
	if (!HasAuthority() || !AbilitySystem)
	{
		return;
	}
	AbilitySystem->InitAbilityActorInfo(this, this);
	AbilitySystem->SetPrimaryStats(Stats);
	AbilitySystem->ApplyStartupEffects(); // derivados + regen (uma vez)
	AbilitySystem->RefillVitals();
}

void AT4CMonster::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	InitAbilitySystem();
}

void AT4CMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilitySystem();
}

void AT4CMonster::TryMeleeAttack()
{
	if (!HasAuthority() || !IsAlive() || !AbilitySystem)
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown)
	{
		return;
	}
	LastAttackTime = Now;

	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(70.f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params);

	const FT4CBalanceConstants Balance;
	const float Damage = BaseDamage * (1.f + Stats.Strength * Balance.DamagePerStrength);

	for (const FHitResult& Hit : Hits)
	{
		if (AT4CCharacter* Target = Cast<AT4CCharacter>(Hit.GetActor()))
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
			{
				FGameplayEffectContextHandle Ctx = AbilitySystem->MakeEffectContext();
				Ctx.AddInstigator(this, this);
				FGameplayEffectSpecHandle Spec = AbilitySystem->MakeOutgoingSpec(UGE_Damage::StaticClass(), 1.f, Ctx);
				if (Spec.IsValid())
				{
					Spec.Data->SetSetByCallerMagnitude(T4CTags::Data_Damage, Damage);
					AbilitySystem->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
				}
				break;
			}
		}
	}
}

void AT4CMonster::HandleDeath(AActor* Killer)
{
	if (bDeadHandled || !HasAuthority())
	{
		return;
	}
	bDeadHandled = true;

	// Concede XP ao jogador que derrotou o monstro.
	if (AT4CCharacter* KillerChar = Cast<AT4CCharacter>(Killer))
	{
		if (AT4CPlayerState* KillerPS = KillerChar->GetPlayerState<AT4CPlayerState>())
		{
			KillerPS->GrantExperience(XPReward);
		}
	}

	// Pede ao GameMode para respawnar um novo monstro neste ponto e dropar loot
	// no local exato da morte.
	if (AT4CGameMode* GM = GetWorld()->GetAuthGameMode<AT4CGameMode>())
	{
		GM->DropLoot(GetActorLocation());
		GM->OnMonsterKilled(SpawnLocation);
	}

	Destroy();
}
