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
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Attach Tool is called"));
	if (!IsToolAlreadyOwned(ToolDefinition))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Not own yet Spawning"));
		AEquippableToolBase* ToolToEquip = GetWorld()->SpawnActor<AEquippableToolBase>(ToolDefinition->ToolAsset, this->GetActorTransform());
		if (!ToolToEquip)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Spawn Failed, Check Tool Asset"));
			return false;
		}
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Tool Spawned"));
		
		UStaticMesh* ToolMesh = ToolDefinition->ToolMesh.IsValid()
		? ToolDefinition->ToolMesh.Get()
		: ToolDefinition->ToolMesh.LoadSynchronous();

		if (ToolMesh && ToolToEquip->ToolMeshComponent)
		{
			ToolToEquip->ToolMeshComponent->SetStaticMesh(ToolMesh);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Tool Assigned"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Tool Assign Failed"));
		}
		
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		
		ToolToEquip->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("RightHandIndex3")));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Attached to socket"));
		
		InventoryComponent->InventoryTool.Add(ToolDefinition);
		ToolToEquip->OwningCharacter = this;
		EquippedTool = ToolToEquip;

		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext((ToolToEquip->ToolMappingContext),1);
			}
			ToolToEquip->BindInputAction(UseAction);
		}return true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Already Owned"));
		return false;
	}
}

bool AJellyCharacterBase::GiveItem(UItemDefinition* ItemDefinition)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("GiveItem Called"));
	if (!ItemDefinition)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Tool is NULL"));
		return false;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Tool is Valid"));
	switch (ItemDefinition->ItemType)
	{
	case EItemType::Tool:
		{	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Item is Tool"));
			UEquippableToolDefinition* ToolDefinition = Cast<UEquippableToolDefinition>(ItemDefinition);
			if (ToolDefinition != nullptr)
			{	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Cast to ToolDefinition succeed"));
				return AttachTool(ToolDefinition);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Cast to ToolDefinition screwed"));
				return false;
			}
		}
	case EItemType::Consumable:
		{GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Item Consumable"));
			return false;
		}
	default:
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("UNKNOWN"));
		return false;
	}

}