#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T4CGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProphecyProgressChanged, float, NewProgress);

/**
 * Estado global do mundo, replicado para todos os clientes.
 * Rastreia o avanço da profecia da "4ª Vinda" — gatilho de eventos globais.
 */
UCLASS()
class T4CREBORN_API AT4CGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Servidor: avança a profecia (0..1). Ao chegar a 1, dispara o evento. */
	void AdvanceProphecy(float Delta);

	UFUNCTION(BlueprintPure, Category = "T4C|World")
	float GetProphecyProgress() const { return ProphecyProgress; }

	UPROPERTY(BlueprintAssignable, Category = "T4C|World")
	FOnProphecyProgressChanged OnProphecyProgressChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ProphecyProgress, VisibleAnywhere, Category = "T4C|World")
	float ProphecyProgress = 0.f;

	UFUNCTION()
	void OnRep_ProphecyProgress();
};
