#include "Player/T4CCharacter.h"
#include "Attributes/T4CAbilityData.h"
#include "Combat/T4CProjectile.h"
#include "Core/T4CPlayerState.h"
#include "GAS/T4CAttributeSet.h"
#include "GAS/T4CGameplayTags.h"
#include "GAS/T4CAbilityInputID.h"
#include "AbilitySystemComponent.h"
#include "Items/T4CInventoryComponent.h"
#include "Items/T4CLootPickup.h"
#include "Items/T4CItemData.h"
#include "NPC/T4CNpc.h"
#include "EngineUtils.h"
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

	// O ASC vive no PlayerState; o Character é apenas o avatar.
	ProjectileClass = AT4CProjectile::StaticClass();
}

void AT4CCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Cor do corpo: azul-aço (distingue jogadores dos monstros vermelhos).
	if (BodyMesh)
	{
		BodyMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(0.2f, 0.45f, 0.95f));
	}
}

UAbilitySystemComponent* AT4CCharacter::GetAbilitySystemComponent() const
{
	if (const AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

UT4CAttributeSet* AT4CCharacter::GetAttributeSet() const
{
	if (const AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		return PS->GetAttributeSet();
	}
	return nullptr;
}

bool AT4CCharacter::IsAlive() const
{
	const UT4CAttributeSet* Set = GetAttributeSet();
	return Set && Set->IsAlive();
}

void AT4CCharacter::InitAbilitySystem()
{
	AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>();
	if (!PS)
	{
		return;
	}
	// Liga o ASC do PlayerState (owner) a este pawn (avatar). Roda no servidor e
	// no cliente — footgun clássico do GAS resolvido aqui.
	if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
	{
		ASC->InitAbilityActorInfo(PS, this);
	}
}

void AT4CCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Servidor: liga o ASC e inicializa os atributos (base + GEs + refill).
	if (HasAuthority())
	{
		InitAbilitySystem();
		if (AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
		{
			PS->InitializeAttributes();
			// Carrega o personagem salvo no 1º spawn (no-op em respawns e sem save).
			PS->LoadCharacterOnce();
		}
		UE_LOG(LogTemp, Display, TEXT("[T4C] %s nasceu em %s"), *GetName(), *GetActorLocation().ToString());
	}
}

void AT4CCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Cliente: o PlayerState (e seu ASC) acabou de chegar; liga o avatar.
	InitAbilitySystem();
}

void AT4CCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AT4CCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AT4CCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AT4CCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AT4CCharacter::LookUpAt);

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

	// Habilidades de classe: Q (slot 0) e E (slot 1). Ativam pelo InputID no ASC.
	PlayerInputComponent->BindAction(TEXT("Ability1"), IE_Pressed, this, &AT4CCharacter::OnAbilityQPressed);
	PlayerInputComponent->BindAction(TEXT("Ability2"), IE_Pressed, this, &AT4CCharacter::OnAbilityEPressed);

	// Recomeçar / trocar de classe (tecla R).
	PlayerInputComponent->BindAction(TEXT("ResetClass"), IE_Pressed, this, &AT4CCharacter::ResetClass);

	// Loot: F coleta o drop próximo (ou interage com NPC); G usa poção; B compra.
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AT4CCharacter::Interact);
	PlayerInputComponent->BindAction(TEXT("UsePotion"), IE_Pressed, this, &AT4CCharacter::UsePotion);
	PlayerInputComponent->BindAction(TEXT("Buy"), IE_Pressed, this, &AT4CCharacter::Buy);
}

// Helper: NPC mais próximo dentro do alcance de interação.
static AT4CNpc* FindNearestNpc(UWorld* World, const FVector& From)
{
	AT4CNpc* Nearest = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (TActorIterator<AT4CNpc> It(World); It; ++It)
	{
		AT4CNpc* Npc = *It;
		if (!Npc) continue;
		const float DistSq = FVector::DistSquared(From, Npc->GetActorLocation());
		const float Reach = Npc->GetInteractRadius();
		if (DistSq <= Reach * Reach && DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Npc;
		}
	}
	return Nearest;
}

void AT4CCharacter::Interact()
{
	ServerInteract();
}

void AT4CCharacter::ServerInteract_Implementation()
{
	// Acha o saco de loot mais próximo dentro do alcance de coleta.
	AT4CLootPickup* Nearest = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	const FVector MyLoc = GetActorLocation();

	for (TActorIterator<AT4CLootPickup> It(GetWorld()); It; ++It)
	{
		AT4CLootPickup* Pickup = *It;
		if (!Pickup || Pickup->IsActorBeingDestroyed())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(MyLoc, Pickup->GetActorLocation());
		const float Reach = Pickup->GetPickupRadius();
		if (DistSq <= Reach * Reach && DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Pickup;
		}
	}

	if (Nearest)
	{
		// Prioridade: coletar loot próximo.
		if (AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
		{
			if (UT4CInventoryComponent* Inv = PS->GetInventory())
			{
				Inv->AddItem(Nearest->GetItem());
				Nearest->Destroy();
			}
		}
		return;
	}

	// Sem loot: interage com o NPC mais próximo (mercador vende / treinador treina).
	AT4CNpc* Npc = FindNearestNpc(GetWorld(), MyLoc);
	AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>();
	if (!Npc || !PS)
	{
		return;
	}
	if (Npc->GetNpcType() == ET4CNpcType::Merchant)
	{
		if (UT4CInventoryComponent* Inv = PS->GetInventory())
		{
			const int32 Earned = Inv->SellAllUnequipped();
			if (Earned > 0)
			{
				PS->GrantGold(Earned);
				UE_LOG(LogTemp, Display, TEXT("[T4C] %s vendeu itens por %d ouro (total %d)"),
					*PS->GetPlayerName(), Earned, PS->GetGold());
			}
		}
	}
	else // Trainer
	{
		PS->TrainPrimaryAttribute();
	}
}

void AT4CCharacter::Buy()
{
	ServerBuy();
}

void AT4CCharacter::ServerBuy_Implementation()
{
	AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>();
	if (!PS)
	{
		return;
	}
	// Precisa estar perto de um mercador.
	AT4CNpc* Npc = FindNearestNpc(GetWorld(), GetActorLocation());
	if (!Npc || Npc->GetNpcType() != ET4CNpcType::Merchant)
	{
		return;
	}

	// Compra uma Poção Maior (da loot table) se tiver ouro.
	FT4CItem Potion;
	for (const FT4CItem& Item : T4CItems::DropTable())
	{
		if (Item.Id == FName(TEXT("potion_major"))) { Potion = Item; break; }
	}
	if (!Potion.IsValid())
	{
		return;
	}
	const int32 Price = Potion.BuyValue();
	if (PS->SpendGold(Price))
	{
		if (UT4CInventoryComponent* Inv = PS->GetInventory())
		{
			Inv->AddItem(Potion);
		}
		UE_LOG(LogTemp, Display, TEXT("[T4C] %s comprou %s por %d ouro (sobra %d)"),
			*PS->GetPlayerName(), *Potion.Name, Price, PS->GetGold());
	}
}

void AT4CCharacter::UsePotion()
{
	ServerUsePotion();
}

void AT4CCharacter::ServerUsePotion_Implementation()
{
	if (AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		if (UT4CInventoryComponent* Inv = PS->GetInventory())
		{
			Inv->UseFirstPotion();
		}
	}
}

void AT4CCharacter::ResetClass()
{
	if (AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		PS->ServerResetClass();
	}
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

void AT4CCharacter::SpawnAttackProjectile(float Damage, FLinearColor Color, float Scale)
{
	if (!ProjectileClass)
	{
		return;
	}

	// Dispara na direção em que o jogador mira (rotação de controle), saindo
	// um pouco à frente da cápsula.
	const FRotator AimRot(0.f, GetControlRotation().Yaw, 0.f);
	const FVector Dir = AimRot.Vector();
	const FVector Muzzle = GetActorLocation() + Dir * 90.f + FVector(0.f, 0.f, 30.f);
	const FTransform SpawnTM(AimRot, Muzzle);

	// Deferred: define dano/cor/escala antes do BeginPlay (replicam como estado inicial).
	if (AT4CProjectile* Projectile = GetWorld()->SpawnActorDeferred<AT4CProjectile>(
		ProjectileClass, SpawnTM, this, this,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
	{
		Projectile->SetDamage(Damage);
		Projectile->SetSource(GetAbilitySystemComponent());
		Projectile->SetVisual(Color, Scale);
		Projectile->FinishSpawning(SpawnTM);
	}
}

void AT4CCharacter::OnAbilityQPressed()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AbilityLocalInputPressed(static_cast<int32>(ET4CAbilityInputID::AbilityQ));
	}
}

void AT4CCharacter::OnAbilityEPressed()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AbilityLocalInputPressed(static_cast<int32>(ET4CAbilityInputID::AbilityE));
	}
}

FString AT4CCharacter::GetAbilityName(int32 Slot) const
{
	if (const AT4CPlayerState* PS = GetPlayerState<AT4CPlayerState>())
	{
		if (PS->HasChosenClass())
		{
			return T4CAbilities::Get(PS->GetChosenClass(), Slot).Name;
		}
	}
	return FString();
}

float AT4CCharacter::GetAbilityCooldownRemaining(int32 Slot) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (Slot < 0 || Slot > 1 || !ASC)
	{
		return 0.f;
	}
	// Consulta o GE de cooldown ativo que carrega a tag do slot (replicado ao dono).
	const FGameplayTag CDTag = (Slot == 1) ? T4CTags::Cooldown_E : T4CTags::Cooldown_Q;
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CDTag));
	float Remaining = 0.f;
	for (const float Time : ASC->GetActiveEffectsTimeRemaining(Query))
	{
		Remaining = FMath::Max(Remaining, Time);
	}
	return Remaining;
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
