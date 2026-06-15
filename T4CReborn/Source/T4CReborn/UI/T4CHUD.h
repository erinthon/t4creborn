#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "T4CHUD.generated.h"

/**
 * HUD desenhado via Canvas (100% C++, sem assets UMG): barras de HP/Mana,
 * nível, XP, atributos e pontos não-gastos, além de flash de dano.
 */
UCLASS()
class T4CREBORN_API AT4CHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawBar(float X, float Y, float W, float H, float Pct,
		const FLinearColor& Fill, const FString& Label);

	float LastHealth = -1.f;
	float HitFlashEndTime = 0.f;
};
