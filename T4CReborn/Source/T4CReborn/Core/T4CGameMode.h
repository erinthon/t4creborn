#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T4CGameMode.generated.h"

/**
 * Regras de partida de Althea. Existe SOMENTE no servidor.
 * Define as classes padrão (Pawn/PlayerState/GameState/Controller).
 */
UCLASS()
class T4CREBORN_API AT4CGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AT4CGameMode();
};
