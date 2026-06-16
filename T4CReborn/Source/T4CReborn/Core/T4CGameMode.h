#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T4CGameMode.generated.h"

class AT4CMonster;
class AT4CLootPickup;

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

	/** Salva o personagem que está saindo (persistência). */
	virtual void Logout(AController* Exiting) override;

	/** Distribui os jogadores entre os PlayerStarts (round-robin), evitando
	 *  que dois nasçam no mesmo ponto e se empurrem. */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** Chamado por um monstro ao morrer; agenda um respawn no mesmo ponto. */
	void OnMonsterKilled(const FVector& SpawnLocation);

	/** Chamado por um monstro ao morrer; rola e dropa loot no local da morte. */
	void DropLoot(const FVector& DeathLocation);

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

	/** Ator de loot dropado pelos monstros (default: AT4CLootPickup). */
	UPROPERTY(EditDefaultsOnly, Category = "T4C|Loot")
	TSubclassOf<AT4CLootPickup> LootPickupClass;

	/** Chance (0..1) de um monstro dropar loot ao morrer. */
	UPROPERTY(EditDefaultsOnly, Category = "T4C|Loot")
	float LootDropChance = 0.65f;

	void SpawnMonster(FVector Location);

	/** Servidor: salva todos os personagens conectados (autosave periódico). */
	void SaveAllPlayers();

	/** Segundos entre autosaves. */
	UPROPERTY(EditDefaultsOnly, Category = "T4C|Persistence")
	float AutoSaveInterval = 30.f;

private:
	int32 NextStartIndex = 0;
	FTimerHandle AutoSaveTimer;

	// --- Harness de auto-teste (headless), ativado por -T4CAutoTest ---
	// Escolhe uma classe para cada jogador e usa Q/E periodicamente, permitindo
	// verificar o pipeline de habilidades sem input manual.
	void RunAutoTest();
	bool bAutoTest = false;
	int32 AutoTestTick = 0;
	FTimerHandle AutoTestTimer;
};
