#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T4CProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPointLightComponent;
class UAbilitySystemComponent;

/**
 * Projétil de ataque: voa em linha reta a partir do jogador e, ao acertar
 * algo com UT4CAttributeComponent, aplica dano. Replicado (servidor manda).
 * Cor e tamanho diferenciam as habilidades visualmente.
 */
UCLASS()
class T4CREBORN_API AT4CProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT4CProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Servidor: define o dano antes do disparo. */
	void SetDamage(float InDamage) { Damage = InDamage; }

	/** Servidor: define o ASC de origem (do atacante) para aplicar o GE de dano. */
	void SetSource(UAbilitySystemComponent* InSourceASC) { SourceASC = InSourceASC; }

	/** Servidor: define cor/tamanho (chamar antes de FinishSpawning). */
	void SetVisual(FLinearColor InColor, float InScale) { ProjColor = InColor; ProjScale = InScale; }

protected:
	virtual void BeginPlay() override;

	void ApplyVisual();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, Category = "T4C")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "T4C")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "T4C")
	TObjectPtr<UPointLightComponent> Light;

	UPROPERTY(VisibleAnywhere, Category = "T4C")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY()
	float Damage = 10.f;

	UPROPERTY(Replicated)
	FLinearColor ProjColor = FLinearColor::White;

	UPROPERTY(Replicated)
	float ProjScale = 0.45f;

	/** ASC de origem (atacante). Weak: o pawn pode morrer antes do impacto. */
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;
};
