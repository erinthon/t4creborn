#include "Combat/T4CProjectile.h"
#include "Attributes/T4CAttributeComponent.h"
#include "Player/T4CCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AT4CProjectile::AT4CProjectile()
{
	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 2.5f;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(22.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SetRootComponent(Collision);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
		Mesh->SetRelativeScale3D(FVector(0.35f));
	}

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->InitialSpeed = 2600.f;
	Movement->MaxSpeed = 2600.f;
	Movement->ProjectileGravityScale = 0.f;
	Movement->bRotationFollowsVelocity = true;
}

void AT4CProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Não colidir com quem disparou.
	if (AActor* Inst = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(Inst, true);
	}

	if (HasAuthority())
	{
		Collision->OnComponentHit.AddDynamic(this, &AT4CProjectile::OnHit);
	}
}

void AT4CProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Sem fogo amigo: projéteis de jogador não ferem outros jogadores.
	const bool bIsPlayer = OtherActor && OtherActor->IsA(AT4CCharacter::StaticClass());

	if (HasAuthority() && OtherActor && OtherActor != GetInstigator() && !bIsPlayer)
	{
		if (UT4CAttributeComponent* Attr = OtherActor->FindComponentByClass<UT4CAttributeComponent>())
		{
			Attr->ApplyDamage(Damage, GetInstigator());
		}
	}

	// Some ao impactar inimigo, parede ou chão.
	Destroy();
}
