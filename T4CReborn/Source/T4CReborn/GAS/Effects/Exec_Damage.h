#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Exec_Damage.generated.h"

/**
 * Execution de dano: lê a magnitude bruta (SetByCaller Data.Damage) e a armadura
 * do alvo, aplica redução plana (mín. 1 de dano) e escreve em IncomingDamage.
 * O AttributeSet converte IncomingDamage em perda de vida em PostGameplayEffectExecute.
 */
UCLASS()
class T4CREBORN_API UExec_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExec_Damage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
