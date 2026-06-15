#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T4CProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

/**
 * Projétil de ataque: voa em linha reta a partir do jogador e, ao acertar
 * algo com UT4CAttributeComponent, aplica dano. Replicado (servidor manda).
 */
UCLASS()
class T4CREBORN_API AT4CProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT4CProjectile();

	/** Servidor: define o dano antes do disparo. */
	void SetDamage(float InDamage) { Damage = InDamage; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, Category = "T4C")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "T4C")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "T4C")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY()
	float Damage = 10.f;
};
