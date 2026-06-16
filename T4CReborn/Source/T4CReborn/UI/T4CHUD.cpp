#include "UI/T4CHUD.h"
#include "Player/T4CCharacter.h"
#include "Core/T4CPlayerState.h"
#include "GAS/T4CAttributeSet.h"
#include "AI/T4CMonster.h"
#include "Attributes/T4CClassData.h"
#include "Items/T4CInventoryComponent.h"
#include "Items/T4CLootPickup.h"
#include "NPC/T4CNpc.h"
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
	UT4CAttributeSet* Attr = Char->GetAttributeSet();
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

	const FLinearColor White(1.f, 1.f, 1.f, 1.f);
	const FLinearColor Gold(1.f, 0.85f, 0.2f, 1.f);
	const FLinearColor Dim(0.75f, 0.75f, 0.8f, 1.f);

	// --- Aguardando o load de persistência (evita o menu de classe 'piscar') ---
	if (!PS->IsLoadResolved())
	{
		DrawText(TEXT("Carregando personagem..."), Gold, W * 0.5f - 130.f, H * 0.45f, nullptr, 1.4f);
		return;
	}

	// --- Menu de seleção de classe (antes de escolher) ---
	if (!PS->HasChosenClass())
	{
		const float PanelW = 560.f;
		const float PanelX = (W - PanelW) * 0.5f;
		float MY = H * 0.22f;
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), PanelX - 20.f, MY - 20.f, PanelW + 40.f, 360.f);
		DrawText(TEXT("ESCOLHA SUA CLASSE  (tecla 1-8)"), Gold, PanelX, MY, nullptr, 1.5f);
		MY += 40.f;
		for (int32 i = 0; i < T4CClasses::Count; ++i)
		{
			const FT4CClassDef Def = T4CClasses::Get(T4CClasses::FromIndex(i));
			const FString Line = FString::Printf(TEXT("[%d] %-12s  STR %2d  END %2d  AGI %2d  INT %2d  WIS %2d"),
				i + 1, *Def.Name, Def.Stats.Strength, Def.Stats.Endurance,
				Def.Stats.Agility, Def.Stats.Intelligence, Def.Stats.Wisdom);
			DrawText(Line, White, PanelX, MY, nullptr, 1.1f);
			MY += 26.f;
		}
		DrawText(TEXT("END aumenta HP | STR aumenta dano | INT/WIS aumentam Mana"),
			Dim, PanelX, MY + 6.f, nullptr, 1.0f);
		return; // esconde o resto do HUD até escolher
	}

	// --- Progressão (canto superior esquerdo) ---
	const FT4CPrimaryStats& S = PS->GetPrimaryStats();
	float TY = 24.f;

	DrawText(FString::Printf(TEXT("[%s]  Nivel %d    XP %d / %d"),
		*PS->GetClassName(), PS->GetCharacterLevel(), PS->GetExperience(), PS->GetXPForNextLevel()),
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
		TY += 24.f;
	}

	// Ouro + pontos de perícia (economia / treinador).
	DrawText(FString::Printf(TEXT("Ouro: %d     Pericia: %d"), PS->GetGold(), PS->GetUnspentSkillPoints()),
		FLinearColor(1.f, 0.85f, 0.2f, 1.f), 30.f, TY, nullptr, 1.1f);

	// --- Inventário e equipamento (canto superior direito) ---
	if (UT4CInventoryComponent* Inv = PS->GetInventory())
	{
		const float PanelW = 300.f;
		const float PX = W - PanelW - 20.f;
		float IY = 24.f;

		DrawText(TEXT("EQUIPAMENTO"), Gold, PX, IY, nullptr, 1.2f);
		IY += 26.f;
		DrawText(FString::Printf(TEXT("Arma: %s (+%.0f dano)"),
			*Inv->GetEquippedWeaponName(), Inv->GetEquippedWeaponDamage()),
			White, PX, IY, nullptr, 1.0f);
		IY += 22.f;
		DrawText(FString::Printf(TEXT("Armadura: %s (+%.0f)"),
			*Inv->GetEquippedArmorName(), Inv->GetEquippedArmor()),
			White, PX, IY, nullptr, 1.0f);
		IY += 30.f;

		const TArray<FT4CItem>& Items = Inv->GetItems();
		DrawText(FString::Printf(TEXT("MOCHILA (%d)"), Items.Num()), Gold, PX, IY, nullptr, 1.2f);
		IY += 26.f;
		for (int32 i = 0; i < Items.Num(); ++i)
		{
			const bool bEquipped = (i == Inv->GetEquippedWeaponIndex()) || (i == Inv->GetEquippedArmorIndex());
			const FString Line = FString::Printf(TEXT("%s %s"),
				bEquipped ? TEXT("[E]") : TEXT(" - "), *Items[i].Name);
			DrawText(Line, Items[i].RarityColor(), PX, IY, nullptr, 1.0f);
			IY += 20.f;
		}
	}

	// --- Prompt de coleta: saco de loot próximo (centro da tela) ---
	{
		AT4CLootPickup* NearestLoot = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		const FVector MyLoc = Char->GetActorLocation();
		for (TActorIterator<AT4CLootPickup> It(GetWorld()); It; ++It)
		{
			AT4CLootPickup* Pickup = *It;
			if (!Pickup) continue;
			const float DistSq = FVector::DistSquared(MyLoc, Pickup->GetActorLocation());
			const float Reach = Pickup->GetPickupRadius();
			if (DistSq <= Reach * Reach && DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				NearestLoot = Pickup;
			}
		}
		if (NearestLoot)
		{
			const FString Prompt = FString::Printf(TEXT("[F] Pegar: %s"), *NearestLoot->GetItem().Name);
			DrawText(Prompt, NearestLoot->GetItem().RarityColor(), W * 0.5f - 110.f, H * 0.62f, nullptr, 1.3f);
		}
	}

	// --- Prompt de NPC próximo (mercador / treinador) ---
	{
		AT4CNpc* NearestNpc = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		const FVector MyLoc = Char->GetActorLocation();
		for (TActorIterator<AT4CNpc> It(GetWorld()); It; ++It)
		{
			AT4CNpc* Npc = *It;
			if (!Npc) continue;
			const float DistSq = FVector::DistSquared(MyLoc, Npc->GetActorLocation());
			const float Reach = Npc->GetInteractRadius();
			if (DistSq <= Reach * Reach && DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				NearestNpc = Npc;
			}
		}
		if (NearestNpc)
		{
			const bool bMerchant = NearestNpc->GetNpcType() == ET4CNpcType::Merchant;
			const FString Prompt = bMerchant
				? TEXT("Mercador  -  [F] Vender tudo   [B] Comprar Pocao")
				: FString::Printf(TEXT("Treinador  -  [F] Treinar (custo %d pericia)"), AT4CPlayerState::TrainCostSkillPoints);
			DrawText(Prompt, FLinearColor(1.f, 0.9f, 0.5f, 1.f), W * 0.5f - 180.f, H * 0.66f, nullptr, 1.3f);
		}
	}

	// --- Barras de vida flutuantes sobre os monstros (feedback de combate) ---
	for (TActorIterator<AT4CMonster> It(GetWorld()); It; ++It)
	{
		AT4CMonster* Monster = *It;
		UT4CAttributeSet* MAttr = Monster ? Monster->GetAttributeSet() : nullptr;
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

	// --- Barra de habilidades (Q / E) no centro inferior ---
	{
		const TCHAR* Keys[2] = { TEXT("Q"), TEXT("E") };
		const float SlotW = 150.f;
		const float SlotH = 40.f;
		const float Gap = 16.f;
		const float TotalW = SlotW * 2.f + Gap;
		float SX = (W - TotalW) * 0.5f;
		const float SY = H - 64.f;

		for (int32 i = 0; i < 2; ++i)
		{
			const FString AbName = Char->GetAbilityName(i);
			const float CD = Char->GetAbilityCooldownRemaining(i);
			const bool bReady = CD <= 0.01f;

			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), SX - 2.f, SY - 2.f, SlotW + 4.f, SlotH + 4.f);
			DrawRect(bReady ? FLinearColor(0.12f, 0.18f, 0.12f, 0.9f) : FLinearColor(0.25f, 0.12f, 0.12f, 0.9f),
				SX, SY, SlotW, SlotH);

			DrawText(FString::Printf(TEXT("[%s] %s"), Keys[i], *AbName),
				bReady ? White : Dim, SX + 8.f, SY + 4.f, nullptr, 1.0f);
			if (!bReady)
			{
				DrawText(FString::Printf(TEXT("%.1fs"), CD), Gold, SX + 8.f, SY + 22.f, nullptr, 1.0f);
			}
			SX += SlotW + Gap;
		}
	}

	// --- Dica de controles (canto inferior direito) ---
	DrawText(TEXT("WASD mover | Mouse olhar | Q/E habilidades | F pegar | G pocao | R trocar classe"),
		FLinearColor(0.8f, 0.8f, 0.8f, 1.f), W - 720.f, H - 24.f, nullptr, 1.0f);
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
