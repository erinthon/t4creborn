#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T4CGameMode.generated.h"

class AT4CMonster;

/**
 * Regras de partida de Althea. Existe SOMENTE no servidor.
 * Define as classes padrão e gerencia o spawn/respawn de monstros (PvE).
 */
UCLASS()
class T4CREBORN_API AT4CGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AT4CGameMode();

	virtual void BeginPlay() override;

	/** Distribui os jogadores entre os PlayerStarts (round-robin), evitando
	 *  que dois nasçam no mesmo ponto e se empurrem. */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** Chamado por um monstro ao morrer; agenda um respawn no mesmo ponto. */
	void OnMonsterKilled(const FVector& SpawnLocation);

	/** Respawna um jogador morto em um PlayerStart. */
	void RespawnPlayer(AController* Controller);

protected:
	/** Classe de monstro a spawnar (default: AT4CMonster). */
	UPROPERTY(EditDefaultsOnly, Category = "T4C|PvE")
	TSubclassOf<AT4CMonster> MonsterClass;

	/** Pontos de spawn dos monstros no mapa de teste. */
	UPROPERTY(EditDefaultsOnly, Category = "T4C|PvE")
	TArray<FVector> MonsterSpawnPoints;

	/** Segundos até um monstro reaparecer após morrer. */
	UPROPERTY(EditDefaultsOnly, Category = "T4C|PvE")
	float MonsterRespawnDelay = 5.f;

	void SpawnMonster(FVector Location);

private:
	int32 NextStartIndex = 0;
};
