#include "Items/T4CLootPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

AT4CLootPickup::AT4CLootPickup()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true; // gira/flutua para chamar atenção

	Root = CreateDefaultSubobject<USphereComponent>(TEXT("Root"));
	Root->InitSphereRadius(40.f);
	Root->SetCollisionEnabled(ECollisionEnabled::NoCollision); // coleta é por consulta do Character
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
		Mesh->SetRelativeScale3D(FVector(0.4f));
		Mesh->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	}

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Root);
	Light->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	Light->SetIntensity(2200.f);
	Light->SetAttenuationRadius(400.f);
	Light->SetCastShadows(false);
}

void AT4CLootPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AT4CLootPickup, Item);
}

void AT4CLootPickup::BeginPlay()
{
	Super::BeginPlay();
	ApplyVisual();
}

void AT4CLootPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Gira lentamente para destacar o drop no chão.
	if (Mesh)
	{
		Mesh->AddLocalRotation(FRotator(0.f, 90.f * DeltaTime, 0.f));
	}
}

void AT4CLootPickup::ApplyVisual()
{
	const FLinearColor Color = Item.RarityColor();
	if (Light)
	{
		Light->SetLightColor(Color);
	}
	if (Mesh)
	{
		Mesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(Color.R, Color.G, Color.B));
	}
}

void AT4CLootPickup::OnRep_Item()
{
	ApplyVisual();
}
