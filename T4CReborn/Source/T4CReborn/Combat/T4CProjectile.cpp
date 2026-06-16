#include "Combat/T4CProjectile.h"
#include "Player/T4CCharacter.h"
#include "GAS/Effects/T4CGameplayEffects.h"
#include "GAS/T4CGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

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
		Mesh->SetRelativeScale3D(FVector(0.45f));
	}

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Collision);
	Light->SetIntensity(3000.f);
	Light->SetAttenuationRadius(450.f);
	Light->SetCastShadows(false);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->InitialSpeed = 2600.f;
	Movement->MaxSpeed = 2600.f;
	Movement->ProjectileGravityScale = 0.f;
	Movement->bRotationFollowsVelocity = true;
}

void AT4CProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AT4CProjectile, ProjColor);
	DOREPLIFETIME(AT4CProjectile, ProjScale);
}

void AT4CProjectile::BeginPlay()
{
	Super::BeginPlay();

	ApplyVisual();

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

void AT4CProjectile::ApplyVisual()
{
	if (Light)
	{
		Light->SetLightColor(ProjColor);
	}
	if (Mesh)
	{
		Mesh->SetRelativeScale3D(FVector(ProjScale));
		// Tinta o material (se expuser parâmetro de cor) para reforçar o elemento.
		Mesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(ProjColor.R, ProjColor.G, ProjColor.B));
	}
}

void AT4CProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Sem fogo amigo: projéteis de jogador não ferem outros jogadores.
	const bool bIsPlayer = OtherActor && OtherActor->IsA(AT4CCharacter::StaticClass());

	if (HasAuthority() && OtherActor && OtherActor != GetInstigator() && !bIsPlayer)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		UAbilitySystemComponent* Src = SourceASC.Get();
		if (TargetASC && Src)
		{
			// GE de dano: instigador = quem disparou (para atribuir XP na morte);
			// a armadura do alvo é mitigada dentro de UExec_Damage.
			FGameplayEffectContextHandle Ctx = Src->MakeEffectContext();
			Ctx.AddInstigator(GetInstigator(), this);
			FGameplayEffectSpecHandle Spec = Src->MakeOutgoingSpec(UGE_Damage::StaticClass(), 1.f, Ctx);
			if (Spec.IsValid())
			{
				Spec.Data->SetSetByCallerMagnitude(T4CTags::Data_Damage, Damage);
				Src->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
			}
		}
	}

	// Some ao impactar inimigo, parede ou chão.
	Destroy();
}
