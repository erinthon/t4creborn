#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T4CPersistenceSubsystem.generated.h"

/** Resultado de um load: bFound + corpo JSON (vazio se não encontrado). */
DECLARE_DELEGATE_TwoParams(FT4COnCharacterLoaded, bool /*bFound*/, const FString& /*Json*/);

/**
 * Camada de transporte da persistência: fala com o serviço HTTP externo
 * (GET/PUT /character?id=<id>). Não conhece regras de jogo — só move JSON.
 * Usado no servidor (listen/dedicated). Base URL vem de DefaultGame.ini
 * ([T4C.Persistence] BaseUrl). Falhas são graciosas (loga e segue).
 */
UCLASS()
class T4CREBORN_API UT4CPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Servidor: grava (PUT) o JSON do personagem no serviço. */
	void SaveCharacter(const FString& Id, const FString& Json);

	/** Servidor: lê (GET) o personagem; OnLoaded(bFound, json) no game thread. */
	void LoadCharacter(const FString& Id, FT4COnCharacterLoaded OnLoaded);

	bool IsConfigured() const { return !BaseUrl.IsEmpty(); }

private:
	FString CharacterUrl(const FString& Id) const;

	FString BaseUrl;
};
