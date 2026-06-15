#include "Core/T4CGameState.h"
#include "Net/UnrealNetwork.h"

void AT4CGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT4CGameState, ProphecyProgress);
}

void AT4CGameState::AdvanceProphecy(float Delta)
{
	if (!HasAuthority())
	{
		return;
	}

	ProphecyProgress = FMath::Clamp(ProphecyProgress + Delta, 0.f, 1.f);
	OnProphecyProgressChanged.Broadcast(ProphecyProgress); // host local

	// TODO (Fase 4): ao atingir 1.0, disparar o evento global da 4ª Vinda.
}

void AT4CGameState::OnRep_ProphecyProgress()
{
	OnProphecyProgressChanged.Broadcast(ProphecyProgress);
}
