#include "AI/T4CMonsterAIController.h"
#include "AI/T4CMonster.h"
#include "Player/T4CCharacter.h"
#include "EngineUtils.h"

AT4CMonsterAIController::AT4CMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AT4CMonsterAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AT4CMonster* Monster = Cast<AT4CMonster>(GetPawn());
	if (!Monster || !Monster->HasAuthority())
	{
		return;
	}

	if (!Monster->IsAlive())
	{
		return;
	}

	// Encontra o jogador vivo mais próximo dentro do raio de aggro.
	const FVector MyLoc = Monster->GetActorLocation();
	AT4CCharacter* Nearest = nullptr;
	float NearestDistSq = AggroRadius * AggroRadius;

	for (TActorIterator<AT4CCharacter> It(GetWorld()); It; ++It)
	{
		AT4CCharacter* Player = *It;
		if (!Player)
		{
			continue;
		}
		if (!Player->IsAlive())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(MyLoc, Player->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = Player;
		}
	}

	if (!Nearest)
	{
		return;
	}

	const FVector ToTarget = Nearest->GetActorLocation() - MyLoc;
	const float Dist = ToTarget.Size();
	const FVector Dir = ToTarget.GetSafeNormal2D();

	if (Dist > Monster->GetAttackRange() * 0.85f)
	{
		// Persegue (esteer manual; CharacterMovement consome o input).
		Monster->AddMovementInput(Dir, 1.f);
	}
	else
	{
		// No alcance: encara o alvo e ataca.
		const FRotator FaceYaw(0.f, Dir.Rotation().Yaw, 0.f);
		Monster->SetActorRotation(FaceYaw);
		Monster->TryMeleeAttack();
	}
}
