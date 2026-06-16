#include "NPC/T4CNpc.h"
#include "Core/T4CVisuals.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

AT4CNpc::AT4CNpc()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	SetRootComponent(BodyMesh);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyMesh->SetCollisionProfileName(TEXT("Pawn"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
		BodyMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.8f));
	}

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(BodyMesh);
	Light->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	Light->SetIntensity(2500.f);
	Light->SetAttenuationRadius(500.f);
	Light->SetCastShadows(false);
}

void AT4CNpc::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AT4CNpc, NpcType);
}

void AT4CNpc::BeginPlay()
{
	Super::BeginPlay();
	ApplyVisual();
}

FString AT4CNpc::GetDisplayName() const
{
	return NpcType == ET4CNpcType::Merchant ? TEXT("Mercador") : TEXT("Treinador");
}

void AT4CNpc::ApplyVisual()
{
	// Mercador = âmbar/ouro; Treinador = azul.
	const FLinearColor Color = (NpcType == ET4CNpcType::Merchant)
		? FLinearColor(1.0f, 0.8f, 0.2f)
		: FLinearColor(0.2f, 0.5f, 1.0f);
	if (Light)
	{
		Light->SetLightColor(Color);
	}
	T4CVisuals::ApplyBodyColor(BodyMesh, this, Color);
}
