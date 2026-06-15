#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "T4CPlayerController.generated.h"

/**
 * Controller do jogador. Mantém input/câmera locais; ações de gameplay
 * são enviadas ao servidor via RPCs no Pawn/PlayerState.
 */
UCLASS()
class T4CREBORN_API AT4CPlayerController : public APlayerController
{
	GENERATED_BODY()
};
