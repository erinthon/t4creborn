#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "T4CCombatant.generated.h"

UINTERFACE(MinimalAPI)
class UT4CCombatant : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implementado por qualquer ator que pode morrer em combate (jogador, monstro).
 * O UT4CAttributeSet chama HandleDeath no avatar quando a vida chega a zero,
 * desacoplando o AttributeSet (que vive no PlayerState ou no pawn) da lógica de
 * morte específica de cada ator (XP, loot, respawn).
 */
class IT4CCombatant
{
	GENERATED_BODY()

public:
	/** Servidor: reage à morte deste ator. Killer pode ser nullptr. */
	virtual void HandleDeath(AActor* Killer) = 0;
};
