#include "AI/T4CMonster.h"
#include "AI/T4CMonsterAIController.h"
#include "Attributes/T4CAttributeComponent.h"
#include "Player/T4CCharacter.h"
#include "Core/T4CPlayerState.h"
#include "Core/T4CGameMode.h"
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

	AttributeComponent = CreateDefaultSubobject<UT4CAttributeComponent>(TEXT("AttributeComponent"));

	AIControllerClass = AT4CMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AT4CMonster::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(this, &AT4CMonster::HandleDeath);
		if (HasAuthority())
		{
			AttributeComponent->RecalculateDerivedStats(Stats, /*bRefill=*/true);
		}
	}
}

void AT4CMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority() && AttributeComponent)
	{
		AttributeComponent->RecalculateDerivedStats(Stats, /*bRefill=*/true);
	}
}

void AT4CMonster::TryMeleeAttack()
{
	if (!HasAuthority() || !AttributeComponent || !AttributeComponent->IsAlive())
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

	const float Damage = BaseDamage * (1.f + Stats.Strength * AttributeComponent->Balance.DamagePerStrength);

	for (const FHitResult& Hit : Hits)
	{
		if (AT4CCharacter* Target = Cast<AT4CCharacter>(Hit.GetActor()))
		{
			if (UT4CAttributeComponent* TargetAttr = Target->GetAttributeComponent())
			{
				TargetAttr->ApplyDamage(Damage, this);
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
