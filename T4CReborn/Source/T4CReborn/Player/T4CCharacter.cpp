#include "Player/T4CCharacter.h"
#include "Attributes/T4CAttributeComponent.h"
#include "Combat/T4CProjectile.h"
#include "Core/T4CPlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Core/T4CGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

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

	ProjectileClass = AT4CProjectile::StaticClass();
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
		UE_LOG(LogTemp, Display, TEXT("[T4C] %s nasceu em %s"), *GetName(), *GetActorLocation().ToString());
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

	// Teclas 1-5: distribuem atributos (após classe) E selecionam classe (antes).
	PlayerInputComponent->BindAction(TEXT("Slot1"), IE_Pressed, this, &AT4CCharacter::AllocStrength);
	PlayerInputComponent->BindAction(TEXT("Slot1"), IE_Pressed, this, &AT4CCharacter::SelectClass0);
	PlayerInputComponent->BindAction(TEXT("Slot2"), IE_Pressed, this, &AT4CCharacter::AllocEndurance);
	PlayerInputComponent->BindAction(TEXT("Slot2"), IE_Pressed, this, &AT4CCharacter::SelectClass1);
	PlayerInputComponent->BindAction(TEXT("Slot3"), IE_Pressed, this, &AT4CCharacter::AllocAgility);
	PlayerInputComponent->BindAction(TEXT("Slot3"), IE_Pressed, this, &AT4CCharacter::SelectClass2);
	PlayerInputComponent->BindAction(TEXT("Slot4"), IE_Pressed, this, &AT4CCharacter::AllocIntelligence);
	PlayerInputComponent->BindAction(TEXT("Slot4"), IE_Pressed, this, &AT4CCharacter::SelectClass3);
	PlayerInputComponent->BindAction(TEXT("Slot5"), IE_Pressed, this, &AT4CCharacter::AllocWisdom);
	PlayerInputComponent->BindAction(TEXT("Slot5"), IE_Pressed, this, &AT4CCharacter::SelectClass4);
	// Teclas 6-8: apenas seleção de classe.
	PlayerInputComponent->BindAction(TEXT("Slot6"), IE_Pressed, this, &AT4CCharacter::SelectClass5);
	PlayerInputComponent->BindAction(TEXT("Slot7"), IE_Pressed, this, &AT4CCharacter::SelectClass6);
	PlayerInputComponent->BindAction(TEXT("Slot8"), IE_Pressed, this, &AT4CCharacter::SelectClass7);
}

void AT4CCharacter::AllocateStat(ET4CAttribute Attribute)
{
	// Só após escolher a classe; o servidor valida os pontos disponíveis.
	if (AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		if (PS->HasChosenClass())
		{
			PS->ServerAllocateStat(Attribute);
		}
	}
}

void AT4CCharacter::SelectClass(int32 Index)
{
	// Só antes de ter escolhido uma classe.
	if (AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		if (!PS->HasChosenClass())
		{
			PS->ServerSelectClass(T4CClasses::FromIndex(Index));
		}
	}
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

	// Cooldown de ataque.
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown)
	{
		return;
	}
	LastAttackTime = Now;

	if (!ProjectileClass)
	{
		return;
	}

	// Dano = ArmaBase * (1 + STR * DamagePerStrength)   (GDD seção 2)
	float StrMultiplier = 1.f;
	if (const AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		StrMultiplier = 1.f + PS->GetPrimaryStats().Strength * AttributeComponent->Balance.DamagePerStrength;
	}
	const float Damage = BaseWeaponDamage * StrMultiplier;

	// Dispara na direção em que o jogador mira (rotação de controle), saindo
	// um pouco à frente da cápsula.
	const FRotator AimRot(0.f, GetControlRotation().Yaw, 0.f);
	const FVector Dir = AimRot.Vector();
	const FVector Muzzle = GetActorLocation() + Dir * 90.f + FVector(0.f, 0.f, 30.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT4CProjectile* Projectile = GetWorld()->SpawnActor<AT4CProjectile>(ProjectileClass, Muzzle, AimRot, SpawnParams))
	{
		Projectile->SetDamage(Damage);
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

	// Desabilita colisão e movimento do pawn morto.
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Respawn do jogador após um atraso (morte não é permanente).
	if (HasAuthority())
	{
		if (AController* C = GetController())
		{
			if (C->IsPlayerController())
			{
				UE_LOG(LogTemp, Display, TEXT("[T4C] %s morreu; respawn em 3s"), *C->GetName());
				TWeakObjectPtr<AController> WeakC = C;
				FTimerHandle Handle;
				FTimerDelegate Del = FTimerDelegate::CreateWeakLambda(this, [this, WeakC]()
				{
					UWorld* World = GetWorld();
					AT4CGameMode* GM = World ? World->GetAuthGameMode<AT4CGameMode>() : nullptr;
					// Remove o pawn morto primeiro, para o respawn nascer limpo num PlayerStart.
					Destroy();
					if (GM && WeakC.IsValid())
					{
						GM->RespawnPlayer(WeakC.Get());
					}
				});
				GetWorldTimerManager().SetTimer(Handle, Del, 3.0f, false);
			}
		}
	}
}
