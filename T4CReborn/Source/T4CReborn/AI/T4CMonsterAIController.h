#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "T4CMonsterAIController.generated.h"

/**
 * IA simples e autoritativa: encontra o jogador vivo mais próximo,
 * persegue por esteer manual (sem NavMesh) e ataca ao alcance.
 */
UCLASS()
class T4CREBORN_API AT4CMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AT4CMonsterAIController();

	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Distância máxima para o monstro notar/perseguir um jogador. */
	UPROPERTY(EditDefaultsOnly, Category = "T4C|AI")
	float AggroRadius = 2500.f;
};
