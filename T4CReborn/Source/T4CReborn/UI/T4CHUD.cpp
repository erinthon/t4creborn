#include "UI/T4CHUD.h"
#include "Player/T4CCharacter.h"
#include "Core/T4CPlayerState.h"
#include "Attributes/T4CAttributeComponent.h"
#include "AI/T4CMonster.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "EngineUtils.h"

void AT4CHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !PlayerOwner)
	{
		return;
	}

	AT4CCharacter* Char = Cast<AT4CCharacter>(PlayerOwner->GetPawn());
	if (!Char)
	{
		return;
	}
	UT4CAttributeComponent* Attr = Char->GetAttributeComponent();
	AT4CPlayerState* PS = PlayerOwner->GetPlayerState<AT4CPlayerState>();
	if (!Attr)
	{
		return;
	}

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Now = GetWorld()->GetTimeSeconds();

	// --- Flash de dano ---
	const float Health = Attr->GetHealth();
	if (LastHealth >= 0.f && Health < LastHealth)
	{
		HitFlashEndTime = Now + 0.35f;
	}
	LastHealth = Health;
	if (Now < HitFlashEndTime)
	{
		const float Alpha = (HitFlashEndTime - Now) / 0.35f * 0.4f;
		DrawRect(FLinearColor(1.f, 0.f, 0.f, Alpha), 0.f, 0.f, W, H);
	}

	// --- Barras (canto inferior esquerdo) ---
	const float BarW = 320.f;
	const float BarH = 22.f;
	const float X = 30.f;
	float Y = H - 90.f;

	const float HealthPct = Attr->GetMaxHealth() > 0.f ? Health / Attr->GetMaxHealth() : 0.f;
	DrawBar(X, Y, BarW, BarH, HealthPct, FLinearColor(0.85f, 0.1f, 0.1f, 1.f),
		FString::Printf(TEXT("HP  %d / %d"), FMath::RoundToInt(Health), FMath::RoundToInt(Attr->GetMaxHealth())));

	Y += BarH + 8.f;
	const float ManaPct = Attr->GetMaxMana() > 0.f ? Attr->GetMana() / Attr->GetMaxMana() : 0.f;
	DrawBar(X, Y, BarW, BarH, ManaPct, FLinearColor(0.15f, 0.35f, 0.9f, 1.f),
		FString::Printf(TEXT("Mana  %d / %d"), FMath::RoundToInt(Attr->GetMana()), FMath::RoundToInt(Attr->GetMaxMana())));

	if (!PS)
	{
		return;
	}

	// --- Progressão (canto superior esquerdo) ---
	const FT4CPrimaryStats& S = PS->GetPrimaryStats();
	float TY = 24.f;
	const FLinearColor White(1.f, 1.f, 1.f, 1.f);
	const FLinearColor Gold(1.f, 0.85f, 0.2f, 1.f);

	DrawText(FString::Printf(TEXT("Nivel %d    XP %d / %d"),
		PS->GetCharacterLevel(), PS->GetExperience(), PS->GetXPForNextLevel()),
		Gold, 30.f, TY, nullptr, 1.3f);
	TY += 28.f;

	DrawText(FString::Printf(TEXT("STR %d   END %d   AGI %d   INT %d   WIS %d"),
		S.Strength, S.Endurance, S.Agility, S.Intelligence, S.Wisdom),
		White, 30.f, TY, nullptr, 1.1f);
	TY += 24.f;

	const int32 Unspent = PS->GetUnspentStatPoints();
	if (Unspent > 0)
	{
		DrawText(FString::Printf(TEXT("Pontos a distribuir: %d  -  [1]STR [2]END [3]AGI [4]INT [5]WIS"), Unspent),
			Gold, 30.f, TY, nullptr, 1.1f);
	}

	// --- Barras de vida flutuantes sobre os monstros (feedback de combate) ---
	for (TActorIterator<AT4CMonster> It(GetWorld()); It; ++It)
	{
		AT4CMonster* Monster = *It;
		UT4CAttributeComponent* MAttr = Monster ? Monster->GetAttributeComponent() : nullptr;
		if (!MAttr || !MAttr->IsAlive())
		{
			continue;
		}

		const FVector Head = Monster->GetActorLocation() + FVector(0.f, 0.f, 130.f);
		const FVector ScreenPos = Project(Head);
		if (ScreenPos.Z <= 0.f)
		{
			continue; // atrás da câmera
		}

		const float MBarW = 70.f;
		const float MBarH = 7.f;
		const float MPct = MAttr->GetMaxHealth() > 0.f ? MAttr->GetHealth() / MAttr->GetMaxHealth() : 0.f;
		const float MX = ScreenPos.X - MBarW * 0.5f;
		const float MY = ScreenPos.Y;
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.7f), MX - 1.f, MY - 1.f, MBarW + 2.f, MBarH + 2.f);
		DrawRect(FLinearColor(0.2f, 0.05f, 0.05f, 0.9f), MX, MY, MBarW, MBarH);
		DrawRect(FLinearColor(0.9f, 0.15f, 0.15f, 1.f), MX, MY, MBarW * FMath::Clamp(MPct, 0.f, 1.f), MBarH);
	}

	// --- Dica de controles (canto inferior direito) ---
	DrawText(TEXT("WASD mover | Mouse olhar | LMB/Espaco atacar"),
		FLinearColor(0.8f, 0.8f, 0.8f, 1.f), W - 470.f, H - 34.f, nullptr, 1.0f);
}

void AT4CHUD::DrawBar(float X, float Y, float W, float H, float Pct,
	const FLinearColor& Fill, const FString& Label)
{
	Pct = FMath::Clamp(Pct, 0.f, 1.f);
	// Fundo
	DrawRect(FLinearColor(0.05f, 0.05f, 0.05f, 0.7f), X - 2.f, Y - 2.f, W + 4.f, H + 4.f);
	DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 0.9f), X, Y, W, H);
	// Preenchimento
	DrawRect(Fill, X, Y, W * Pct, H);
	// Rótulo
	DrawText(Label, FLinearColor::White, X + 8.f, Y + 2.f, nullptr, 1.0f);
}
