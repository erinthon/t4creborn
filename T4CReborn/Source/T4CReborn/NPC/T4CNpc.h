#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T4CNpc.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Papel do NPC na vila. */
UENUM(BlueprintType)
enum class ET4CNpcType : uint8
{
	Merchant UMETA(DisplayName = "Mercador"),
	Trainer  UMETA(DisplayName = "Treinador")
};

/**
 * NPC parado da vila (mercador/treinador). Replicado para os clientes verem o
 * corpo e o HUD mostrar o prompt. A interação é por proximidade: o AT4CCharacter
 * acha o NPC mais próximo e chama o servidor (mesmo padrão do loot).
 */
UCLASS()
class T4CREBORN_API AT4CNpc : public AActor
{
	GENERATED_BODY()

public:
	AT4CNpc();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Servidor: define o tipo antes do FinishSpawning. */
	void SetNpcType(ET4CNpcType InType) { NpcType = InType; }

	ET4CNpcType GetNpcType() const { return NpcType; }
	float GetInteractRadius() const { return InteractRadius; }
	FString GetDisplayName() const;

protected:
	virtual void BeginPlay() override;
	void ApplyVisual();

	UFUNCTION()
	void OnRep_NpcType() { ApplyVisual(); }

	UPROPERTY(VisibleAnywhere, Category = "T4C") TObjectPtr<UStaticMeshComponent> BodyMesh;
	UPROPERTY(VisibleAnywhere, Category = "T4C") TObjectPtr<UPointLightComponent> Light;

	UPROPERTY(ReplicatedUsing = OnRep_NpcType, EditAnywhere, Category = "T4C")
	ET4CNpcType NpcType = ET4CNpcType::Merchant;

	UPROPERTY(EditDefaultsOnly, Category = "T4C")
	float InteractRadius = 260.f;
};
