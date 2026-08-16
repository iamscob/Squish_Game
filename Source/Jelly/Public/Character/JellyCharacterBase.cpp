// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyCharacterBase.h"
#include "EnhancedInputSubsystems.h"

#include "InventoryComponent.h"
#include "JellyStatusComponent.h"
#include "JelloCombatComponent.h"
#include "EquippableToolBase.h"
#include "EquippableToolDefinition.h"





// Sets default values
AJellyCharacterBase::AJellyCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
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
		EnhancedInputComponent->BindAction(DropAction,ETriggerEvent::Started, this, &AJellyCharacterBase::Drop);
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
		ToolToEquip->ToolMeshComponent->SetRelativeScale3D(FVector(2.f,2.f,2.f));
	
		ToolToEquip->OwningCharacter = this;
		EquippedTool = ToolToEquip;

		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				if (ToolToEquip->ToolMappingContext){
					Subsystem->AddMappingContext((ToolToEquip->ToolMappingContext),1);
				}
			}
		}
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
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("NoTool"));
		return;
	}
	RemoveToolMappingContext(EquippedTool);
	FRotator CharacterRotator = GetActorRotation();
	FRotator ThrowRotator(0.f, CharacterRotator.Yaw,0.f);
	FVector ThrowDirection = ThrowRotator.Vector();

	EquippedTool->Thrower = this;
	EquippedTool->bThrowerWasChasing = StatusComponent && StatusComponent->IsChasing();
	
	EquippedTool->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	FVector ThrowStart = EquippedTool->GetActorLocation() + (ThrowDirection * 150.f);
	EquippedTool->SetActorLocation(ThrowStart);
	
	EquippedTool->ToolMeshComponent->SetWorldScale3D(FVector(EquippedTool->WorldScale));
	
	EquippedTool->ToolMeshComponent->SetSimulatePhysics(true);
	EquippedTool->ToolMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	EquippedTool->ToolMeshComponent->SetCollisionProfileName("PhysicsActor");
	
	
	float LaunchForce = 1800.f;
	EquippedTool->ToolMeshComponent->AddImpulse(ThrowDirection * LaunchForce, NAME_None, true);
	
	EquippedTool = nullptr;
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


