#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/T4CItemData.h"
#include "T4CLootPickup.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class USphereComponent;

/**
 * Saco de loot caído no mundo, contendo um FT4CItem. Replicado (servidor manda).
 * O jogador coleta aproximando-se e pressionando Interagir (F); o AT4CCharacter
 * faz a consulta de proximidade e chama o servidor. Brilha com a cor da raridade.
 */
UCLASS()
class T4CREBORN_API AT4CLootPickup : public AActor
{
	GENERATED_BODY()

public:
	AT4CLootPickup();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	/** Servidor: define o item carregado (chamar antes de FinishSpawning). */
	void SetItem(const FT4CItem& InItem) { Item = InItem; }

	const FT4CItem& GetItem() const { return Item; }

	/** Raio dentro do qual o jogador consegue coletar. */
	float GetPickupRadius() const { return PickupRadius; }

protected:
	virtual void BeginPlay() override;

	void ApplyVisual();

	UPROPERTY(VisibleAnywhere, Category = "T4C") TObjectPtr<USphereComponent> Root;
	UPROPERTY(VisibleAnywhere, Category = "T4C") TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, Category = "T4C") TObjectPtr<UPointLightComponent> Light;

	UPROPERTY(ReplicatedUsing = OnRep_Item) FT4CItem Item;

	UFUNCTION()
	void OnRep_Item();

	UPROPERTY(EditDefaultsOnly, Category = "T4C") float PickupRadius = 220.f;
};
