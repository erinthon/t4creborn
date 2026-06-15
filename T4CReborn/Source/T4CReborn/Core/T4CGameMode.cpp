#include "Core/T4CGameMode.h"
#include "Core/T4CGameState.h"
#include "Core/T4CPlayerState.h"
#include "Player/T4CCharacter.h"
#include "Player/T4CPlayerController.h"
#include "AI/T4CMonster.h"
#include "UI/T4CHUD.h"
#include "TimerManager.h"

AT4CGameMode::AT4CGameMode()
{
	DefaultPawnClass = AT4CCharacter::StaticClass();
	PlayerControllerClass = AT4CPlayerController::StaticClass();
	PlayerStateClass = AT4CPlayerState::StaticClass();
	GameStateClass = AT4CGameState::StaticClass();
	HUDClass = AT4CHUD::StaticClass();

	MonsterClass = AT4CMonster::StaticClass();

	// Pontos de spawn espalhados pela arena de teste (chão ~5000x5000).
	MonsterSpawnPoints = {
		FVector(800.f, 0.f, 120.f),
		FVector(-800.f, 400.f, 120.f),
		FVector(400.f, -900.f, 120.f),
		FVector(-600.f, -600.f, 120.f)
	};
}

void AT4CGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (const FVector& Point : MonsterSpawnPoints)
	{
		SpawnMonster(Point);
	}
}

void AT4CGameMode::SpawnMonster(FVector Location)
{
	if (!MonsterClass)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AT4CMonster* Spawned = GetWorld()->SpawnActor<AT4CMonster>(MonsterClass, Location, FRotator::ZeroRotator, Params);
	UE_LOG(LogTemp, Display, TEXT("[T4C] Monstro spawnado em %s (%s)"),
		*Location.ToString(), Spawned ? TEXT("ok") : TEXT("FALHOU"));
}

void AT4CGameMode::OnMonsterKilled(const FVector& SpawnLocation)
{
	// Respawn após um atraso, mantendo a população de monstros.
	FTimerHandle Handle;
	FTimerDelegate Del = FTimerDelegate::CreateUObject(this, &AT4CGameMode::SpawnMonster, SpawnLocation);
	GetWorldTimerManager().SetTimer(Handle, Del, MonsterRespawnDelay, false);
}
