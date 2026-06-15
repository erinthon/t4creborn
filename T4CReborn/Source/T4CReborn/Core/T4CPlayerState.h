#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Attributes/T4CAttributeData.h"
#include "T4CPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatsChanged);

/**
 * Dados persistentes do personagem: os 5 atributos, nível, XP e pontos
 * não-gastos. Replicado para todos. Mutações ocorrem SOMENTE no servidor.
 */
UCLASS()
class T4CREBORN_API AT4CPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AT4CPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	const FT4CPrimaryStats& GetPrimaryStats() const { return PrimaryStats; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	int32 GetCharacterLevel() const { return CharacterLevel; }

	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	int32 GetUnspentStatPoints() const { return UnspentStatPoints; }

	/** Servidor: concede XP e processa level-ups. */
	void GrantExperience(int32 Amount);

	/** Cliente → servidor: gasta 1 ponto não-alocado num atributo. */
	UFUNCTION(Server, Reliable)
	void ServerAllocateStat(ET4CAttribute Attribute);

	/** XP necessário para alcançar o próximo nível. */
	UFUNCTION(BlueprintPure, Category = "T4C|Progression")
	int32 GetXPForNextLevel() const;

	UPROPERTY(BlueprintAssignable, Category = "T4C|Progression")
	FOnStatsChanged OnStatsChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	FT4CPrimaryStats PrimaryStats;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	int32 CharacterLevel = 1;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	int32 Experience = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	int32 UnspentStatPoints = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Progression, VisibleAnywhere, Category = "T4C|Progression")
	int32 UnspentSkillPoints = 0;

	UFUNCTION()
	void OnRep_Progression();

	/** Servidor: avança um nível e concede pontos (fiel ao T4C: 5 stat / 15 skill). */
	void LevelUp();

	/** Pede ao Character/AttributeComponent para recalcular HP/Mana. Servidor. */
	void PushDerivedStatsToPawn(bool bRefill);

	// Pontos concedidos por nível — espelham FT4CBalanceConstants.
	static constexpr int32 StatPointsPerLevel = 5;
	static constexpr int32 SkillPointsPerLevel = 15;
};
