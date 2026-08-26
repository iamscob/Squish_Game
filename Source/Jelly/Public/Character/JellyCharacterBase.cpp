// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "InventoryComponent.h"
#include "JellyStatusComponent.h"
#include "JelloCombatComponent.h"
#include "EquippableToolBase.h"
#include "EquippableToolDefinition.h"
#include "JellyPlayerState.h"
#include "Materials/MaterialInstanceDynamic.h"





// Sets default values
AJellyCharacterBase::AJellyCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	SetReplicateMovement(true);
	
	// Setting Camera Boom Up
	CameraBoom=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	check(CameraBoom!=nullptr);
	CameraBoom->SetupAttachment((GetRootComponent()));
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeLocation(CameraOffset);
	
	
	// Setting Up Camera Component
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	check(FollowCamera!=nullptr);
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->FieldOfView = FieldOfView;
	
	// Setting Up Components
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	StatusComponent = CreateDefaultSubobject<UJellyStatusComponent>(TEXT("StatusComponent"));
	CombatComponent = CreateDefaultSubobject<UJelloCombatComponent>(TEXT("CombatComponent"));
	

}



// Called when the game starts or when spawned
void AJellyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	check(GEngine != nullptr);
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ThirdPersonContext, 0);
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f,FColor::Black, TEXT("We're using Jello now."));
	
	ApplyPlayerColor();
}

// Called every frame
void AJellyCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input 
void AJellyCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent *EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AJellyCharacterBase::Move);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AJellyCharacterBase::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AJellyCharacterBase::StopJumping);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,this,&AJellyCharacterBase::Look);
		
		EnhancedInputComponent->BindAction(ThrowAction,ETriggerEvent::Started, this, &AJellyCharacterBase::Throw);
		EnhancedInputComponent->BindAction(MeleeAction,ETriggerEvent::Started,this, &AJellyCharacterBase::MeleeAttack);
	}
	
}
// Input Actions Implementation
void AJellyCharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MovementValue = Value.Get<FVector2D>();
	if (Controller)
	{
	 const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
        AddMovementInput(ForwardDirection, MovementValue.Y);
        AddMovementInput(RightDirection, MovementValue.X);
		
		
	}
}
	
	// Camera Look Implementation
void AJellyCharacterBase::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
		
	if (Controller)
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

	// Tool Attachment
bool AJellyCharacterBase::AttachTool(UEquippableToolDefinition* ToolDefinition)
{
	if (!HasAuthority()) return false;

	if (!ToolDefinition) return false;
	if (EquippedTool) return false;
	if (!ToolDefinition->ToolAsset) return false;
	UWorld* World = GetWorld();
	if (!World) return false;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	AEquippableToolBase* ToolToEquip = World->SpawnActor<AEquippableToolBase>
	(ToolDefinition->ToolAsset,
		GetActorTransform(),
		SpawnParameters);
	if (!ToolToEquip) return false;
			
		UStaticMesh* ToolMesh = ToolDefinition->ToolMesh.IsValid()
		? ToolDefinition->ToolMesh.Get()
		: ToolDefinition->ToolMesh.LoadSynchronous();

		if (!ToolMesh || !ToolToEquip->ToolMeshComponent)
		{
			ToolToEquip->Destroy();
			return false;
		}
	ToolToEquip->ToolMeshComponent->SetStaticMesh(ToolMesh);
	
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	
		
		ToolToEquip->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("RightHandIndex3")));
		ToolToEquip->SetActorRelativeScale3D(FVector(2.f));
	
		ToolToEquip->OwningCharacter = this;
		EquippedTool = ToolToEquip;
	AddToolMappingContext(ToolToEquip);
	
	ForceNetUpdate();
	ToolToEquip->ForceNetUpdate();
	return true;
	}

// PickUps Separation
bool AJellyCharacterBase::GiveItem(UItemDefinition* ItemDefinition)
{
	
	if (!ItemDefinition)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Tool is NULL"));
		return false;
	}
	
	
	switch (ItemDefinition->ItemType)
	{
	case EItemType::Tool:
		{
			UEquippableToolDefinition* ToolDefinition = Cast<UEquippableToolDefinition>(ItemDefinition);
			if (ToolDefinition != nullptr)
			{	
				return AttachTool(ToolDefinition);
			}
			else
			{
				
				return false;
			}
		}
	case EItemType::Consumable:
		{
			return true;
		}
	default:
		
		return false;
	}
	
}

void AJellyCharacterBase::RemoveToolMappingContext(AEquippableToolBase* Tool)
{
	{
		if (!Tool || !Tool->ToolMappingContext) return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController) return;
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::
	GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!Subsystem) return;
	Subsystem->RemoveMappingContext(Tool->ToolMappingContext);
}

	
	
	
// Throw Tool fun
void AJellyCharacterBase::Throw()
{
	if (!EquippedTool)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("NoTool"));
			return;
		}
	}
	RemoveToolMappingContext(EquippedTool);
	
	if (HasAuthority())
	{
		PerformThrow();
		return;
	}
	ServerThrow();

}


//DropTool Function
void AJellyCharacterBase::Drop()
{
	if (!EquippedTool)
	{
		return;
	}
	RemoveToolMappingContext(EquippedTool);
	
	FVector DropDirection = FVector(0, 0, -1);
	EquippedTool->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	FRotator CharacterRotation = GetActorRotation();
	FVector ForwardVector = FRotator(.0f, CharacterRotation.Yaw, .0f).Vector();
	FVector ThrowStart = EquippedTool->GetActorLocation()+ (ForwardVector * 100.f);
	EquippedTool->SetActorLocation(ThrowStart);
	
	EquippedTool->ToolMeshComponent->SetWorldScale3D(FVector(EquippedTool->WorldScale));
	
	EquippedTool->ToolMeshComponent->SetSimulatePhysics(true);
	EquippedTool->ToolMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	EquippedTool->ToolMeshComponent->SetCollisionProfileName("PhysicsActor");
	float LaunchForce = 50.f;
	EquippedTool->ToolMeshComponent->AddImpulse(DropDirection * LaunchForce, NAME_None, true);
	
	EquippedTool = nullptr;

}

void AJellyCharacterBase::MeleeAttack()
{
	if (CombatComponent)
	{
		CombatComponent->MeleeAttack();
	}
}

void AJellyCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ApplyPlayerColor();
}

void AJellyCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	ApplyPlayerColor();
}

void AJellyCharacterBase::ApplyPlayerColor()
{
	const AJellyPlayerState* JellyPlayerState = GetPlayerState<AJellyPlayerState>();

	if (!JellyPlayerState || !GetMesh()) return;
	
	const uint8 ColorIndex = JellyPlayerState->GetPlayerColorIndex();
	
	static const FLinearColor PlayerColors[] ={
		FLinearColor(1.f, .15f,.22f,1.f),
		FLinearColor(1.f, .48f,.08f,1.f),
		FLinearColor(.3f, 1.f,.18f,1.f),
		FLinearColor(.05f, .9f,.85f,1.f),
		FLinearColor(.12f, .35f,1.f,1.f),
		FLinearColor(.65f, .12f,1.f,1.f)
	};
	constexpr uint8 ColorCount = UE_ARRAY_COUNT(PlayerColors);

	if (ColorIndex >= ColorCount) return;

	if (!PlayerColorMaterial)
	{
		PlayerColorMaterial = GetMesh()->CreateDynamicMaterialInstance(PlayerColorMaterialIndex);
	}
	PlayerColorMaterial->SetVectorParameterValue(PlayerColorParameterName, PlayerColors[ColorIndex]);
}

bool AJellyCharacterBase::HasEquippedTool() const
{
	return IsValid(EquippedTool);
}

void AJellyCharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AJellyCharacterBase, EquippedTool);
}

void AJellyCharacterBase::AddToolMappingContext(AEquippableToolBase* Tool)
{
	if (!Tool || !Tool->ToolMappingContext) return;
	
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController) return;
	
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	
	if (!LocalPlayer) return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::
	GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	if (!Subsystem) return;
	Subsystem->AddMappingContext(Tool->ToolMappingContext,1);
}

void AJellyCharacterBase::OnRep_EquippedTool()
{
	if (LocallyMappedTool.IsValid())
	{
		RemoveToolMappingContext(LocallyMappedTool.Get());
	}
	LocallyMappedTool = EquippedTool;

	if (EquippedTool)
	{
		EquippedTool->SetActorRelativeScale3D(FVector(2.f));
		AddToolMappingContext(EquippedTool);
	}
}

void AJellyCharacterBase::ServerThrow_Implementation()
{
	PerformThrow();
}

void AJellyCharacterBase::PerformThrow()
{
	if (!HasAuthority() || !EquippedTool || !EquippedTool->ToolMeshComponent) return;
	
	AEquippableToolBase* ToolToThrow = EquippedTool;
	
	const FRotator CharacterRotator = GetActorRotation();
	const FRotator ThrowRotator(0.f, CharacterRotator.Yaw,0.f);
	const FVector ThrowDirection = ThrowRotator.Vector();
	
	ToolToThrow->Thrower = this;
	ToolToThrow->bThrowerWasChasing = StatusComponent && StatusComponent->IsChasing();
	
	ToolToThrow->OwningCharacter = nullptr;
	ToolToThrow->SetOwner(nullptr);
	
	const FVector ThrowStart = EquippedTool->GetActorLocation() + (ThrowDirection * 150.f);
	ToolToThrow->MulticastPrepareForThrow(ThrowStart);
	
	ToolToThrow->StartPickupCooldown();
	
	constexpr float LaunchForce = 1800.f;
	ToolToThrow->ToolMeshComponent->AddImpulse(ThrowDirection * LaunchForce, NAME_None, true);
	
	EquippedTool = nullptr;
	
	ForceNetUpdate();
	ToolToThrow->ForceNetUpdate();
}

bool AJellyCharacterBase::AttachExistingTool(AEquippableToolBase* ToolToEquip)
{
	if (!HasAuthority() || !ToolToEquip || EquippedTool) return false;
	
	ToolToEquip->SetOwner(this);
	ToolToEquip->SetInstigator(this);
	ToolToEquip->OwningCharacter = this;
	ToolToEquip->Thrower = nullptr;
	ToolToEquip->ResetProcessedHit();
	
	ToolToEquip->MulticastPrepareForHeld(this);
	
	EquippedTool = ToolToEquip;
	
	AddToolMappingContext(ToolToEquip);
	
	ForceNetUpdate();
	ToolToEquip->ForceNetUpdate();
	
	return true;
}
