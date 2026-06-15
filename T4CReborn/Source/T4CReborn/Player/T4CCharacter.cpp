#include "Player/T4CCharacter.h"
#include "Attributes/T4CAttributeComponent.h"
#include "Core/T4CPlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AT4CCharacter::AT4CCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// O personagem orienta-se pela direção do movimento (estilo action-RPG).
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	// Corpo visível: cilindro padrão do engine (sempre disponível, sem conteúdo de projeto).
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
		// Cilindro do engine tem 100uu de altura; a cápsula tem ~176uu (half height 88).
		BodyMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f)); // visão ligeiramente isométrica

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AttributeComponent = CreateDefaultSubobject<UT4CAttributeComponent>(TEXT("AttributeComponent"));
}

void AT4CCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(this, &AT4CCharacter::HandleDeath);
	}
}

void AT4CCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Servidor: inicializa HP/Mana a partir dos atributos do PlayerState.
	if (HasAuthority())
	{
		if (AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
		{
			if (AttributeComponent)
			{
				AttributeComponent->RecalculateDerivedStats(PS->GetPrimaryStats(), /*bRefill=*/true);
			}
		}
	}
}

void AT4CCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AT4CCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AT4CCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AT4CCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AT4CCharacter::LookUpAt);
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &AT4CCharacter::Attack);
}

void AT4CCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Forward, Value);
	}
}

void AT4CCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Right, Value);
	}
}

void AT4CCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AT4CCharacter::LookUpAt(float Value)
{
	AddControllerPitchInput(Value);
}

void AT4CCharacter::Attack()
{
	// Input é local; a resolução do golpe acontece no servidor.
	ServerAttack();
}

void AT4CCharacter::ServerAttack_Implementation()
{
	if (!AttributeComponent || !AttributeComponent->IsAlive())
	{
		return;
	}

	// Esfera à frente do personagem para encontrar um alvo.
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * MeleeRange;

	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(60.f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params);

	// Dano = ArmaBase * (1 + STR * DamagePerStrength)   (GDD seção 2)
	float StrMultiplier = 1.f;
	if (const AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		StrMultiplier = 1.f + PS->GetPrimaryStats().Strength * AttributeComponent->Balance.DamagePerStrength;
	}
	const float Damage = BaseWeaponDamage * StrMultiplier;

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == this)
		{
			continue;
		}

		if (AT4CCharacter* Target = Cast<AT4CCharacter>(HitActor))
		{
			if (UT4CAttributeComponent* TargetAttr = Target->GetAttributeComponent())
			{
				TargetAttr->ApplyDamage(Damage, this);
				break; // um alvo por golpe nesta primeira iteração
			}
		}
	}
}

void AT4CCharacter::HandleDeath(AActor* Killer)
{
	if (bDeadHandled)
	{
		return;
	}
	bDeadHandled = true;

	// Servidor concede XP ao matador (se for outro jogador).
	if (HasAuthority())
	{
		if (AT4CCharacter* KillerChar = Cast<AT4CCharacter>(Killer))
		{
			if (AT4CPlayerState* KillerPS = KillerChar->GetPlayerState<AT4CPlayerState>())
			{
				KillerPS->GrantExperience(50);
			}
		}
	}

	// Desabilita colisão e movimento; respawn fica para o GameMode (Fase 2).
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
