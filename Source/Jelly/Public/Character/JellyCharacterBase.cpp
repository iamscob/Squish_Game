// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimBlueprint.h"
#include "InventoryComponent.h"
#include "EquippableToolBase.h"
#include "EquippableToolDefinition.h"



// Sets default values
AJellyCharacterBase::AJellyCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
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
	
	// Setting Up Inventory Component
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	

}



// Called when the game starts or when spawned
void AJellyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	check(GEngine != nullptr);
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ThirdPersonContext, 0);
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f,FColor::Black, TEXT("We're using JelloCharacter now."));
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


	// Check if tool is already owned
bool AJellyCharacterBase::IsToolAlreadyOwned(UEquippableToolDefinition* ToolDefinition)
{
	for (UEquippableToolDefinition* InventoryItem : InventoryComponent->InventoryTool)
	{
		if (ToolDefinition->ID.EqualTo(InventoryItem->ID))
		{
			return true;
		}
	}
	return false;
}


	// Tool Attachment
bool AJellyCharacterBase::AttachTool(UEquippableToolDefinition* ToolDefinition)
{
	
	if (!IsToolAlreadyOwned(ToolDefinition))
	{
		
		AEquippableToolBase* ToolToEquip = GetWorld()->SpawnActor<AEquippableToolBase>(ToolDefinition->ToolAsset, this->GetActorTransform());
		if (!ToolToEquip)
		{
			
			return false;
		}
		
		
		UStaticMesh* ToolMesh = ToolDefinition->ToolMesh.IsValid()
		? ToolDefinition->ToolMesh.Get()
		: ToolDefinition->ToolMesh.LoadSynchronous();

		if (ToolMesh && ToolToEquip->ToolMeshComponent)
		{
			ToolToEquip->ToolMeshComponent->SetStaticMesh(ToolMesh);
			
		}
		else
		{
			
		}
		
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		ToolToEquip->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("RightHandIndex3")));
		ToolToEquip->ToolMeshComponent->SetRelativeScale3D(FVector(2.f,2.f,2.f));
		
		
		
		InventoryComponent->InventoryTool.Add(ToolDefinition);
		ToolToEquip->OwningCharacter = this;
		EquippedTool = ToolToEquip;

		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext((ToolToEquip->ToolMappingContext),1);
			}
		}
		return true;
	}
	else
	{
		
		return false;
	}
}

// PickUps separation
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


// Throw Tool fun
void AJellyCharacterBase::Throw()
{
	if (!EquippedTool)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("NoTool"));
		return;
	}
	FRotator CharacterRotator = GetActorRotation();
	FRotator ThrowRotator(0.f, CharacterRotator.Yaw,0.f);
	FVector ThrowDirection = ThrowRotator.Vector();

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


//DropTool fun
void AJellyCharacterBase::Drop()
{
	if (!EquippedTool)
	{
		return;
	}
	
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
