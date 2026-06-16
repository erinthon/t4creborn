#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Attributes/T4CAttributeData.h"
#include "Attributes/T4CClassData.h"
#include "Items/T4CItemData.h"
#include "T4CSaveGame.generated.h"

/**
 * Snapshot persistente de um personagem em disco (Saved/SaveGames/<slot>.sav).
 * Espelha os dados duráveis do AT4CPlayerState + inventário. Versão de schema
 * incluída para migração futura. Passo intermediário rumo a dedicated server + DB.
 */
UCLASS()
class T4CREBORN_API UT4CSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() int32 SaveVersion = 1;

	UPROPERTY() bool bHasChosenClass = false;
	UPROPERTY() ET4CClass ChosenClass = ET4CClass::Warrior;
	UPROPERTY() FT4CPrimaryStats PrimaryStats;

	UPROPERTY() int32 CharacterLevel = 1;
	UPROPERTY() int32 Experience = 0;
	UPROPERTY() int32 UnspentStatPoints = 0;
	UPROPERTY() int32 UnspentSkillPoints = 0;

	UPROPERTY() TArray<FT4CItem> Items;
	UPROPERTY() int32 EquippedWeaponIndex = -1;
	UPROPERTY() int32 EquippedArmorIndex = -1;
};
