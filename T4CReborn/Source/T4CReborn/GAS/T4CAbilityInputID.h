#pragma once

#include "CoreMinimal.h"

/**
 * IDs de input das habilidades. O valor é guardado no FGameplayAbilitySpec ao
 * conceder a habilidade; o Character ativa via ASC->AbilityLocalInputPressed(ID).
 */
UENUM()
enum class ET4CAbilityInputID : uint8
{
	None      = 0,
	AbilityQ  = 1, // slot 0
	AbilityE  = 2  // slot 1
};
