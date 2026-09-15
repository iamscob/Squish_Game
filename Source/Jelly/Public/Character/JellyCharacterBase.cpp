// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyCharacterBase.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "InventoryComponent.h"
#include "JellyStatusComponent.h"
#include "JelloCombatComponent.h"
#include "EquippableToolBase.h"
#include "EquippableToolDefinition.h"
#include "JellyPlayerState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "JellyGameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DashComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"



// Sets default values
AJellyCharacterBase::AJellyCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
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
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	PlayerRingDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("PlayerDecalComponent"));
	ThrowAimDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("ThrowAimDecal"));
	
	PlayerRingDecal->SetupAttachment(GetRootComponent());
	PlayerRingDecal->SetRelativeRotation(FRotator(-90.f,0.f, 0.f));
	PlayerRingDecal->DecalSize = FVector(300.f, 70.f, 70.f);
	PlayerRingDecal->FadeScreenSize = 0.001f;
	PlayerRingDecal->SortOrder = 10;
	PlayerRingDecal->SetVisibility(false);
	ThrowAimDecal->SetupAttachment(GetRootComponent());
	ThrowAimDecal->SetRelativeRotation(FRotator(-90.f,0.f,90.f));
	ThrowAimDecal->DecalSize = FVector(300.f,70.f, 50.f);
	ThrowAimDecal->FadeScreenSize = 0.001f;
	ThrowAimDecal->SortOrder = 9;
	ThrowAimDecal->SetVisibility(false);
	
	
	

}



// Called when the game starts or when spawned
void AJellyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	check(GEngine != nullptr);

	if (GetCapsuleComponent())
	{	
		const float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		const float GroundOffset = -CapsuleHalfHeight + 8.f;

		if (PlayerRingDecal)
		{
			PlayerRingDecal->SetRelativeLocation(FVector(0.f, 0.f, GroundOffset));
		}

		if (ThrowAimDecal)
		{
			ThrowAimDecal->SetRelativeLocation(FVector(105.f, 0.f, GroundOffset + 1.f));
		}
	}
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ThirdPersonContext, 0);
		}
	}

	
}

void AJellyCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsPreparingThrow || !ThrowAimDecal)
	{
		SetActorTickEnabled(false);
		return;
	}
	
	constexpr float RevealDuration = .18f;
	constexpr float StartLengthScale = .12f;
	
	ThrowAnimAlpha = FMath::Clamp(ThrowAnimAlpha + DeltaTime / RevealDuration, 0.f, 1.f);
	
	const float EasedAlpha = FMath::InterpEaseOut(0.f,1.f, ThrowAnimAlpha, 3.f);
	
	const float LengthScale = FMath::Lerp(StartLengthScale, 1.f, EasedAlpha);
	
	ThrowAimDecal->SetRelativeScale3D(FVector(1.f,1.f,LengthScale));
	
	FVector ArrowLocation = ThrowAimDecal->GetRelativeLocation();
	
	ArrowLocation.X = 55.f + 50.f * LengthScale;
	
	ThrowAimDecal->SetRelativeLocation(ArrowLocation);

	if (ThrowAnimAlpha >= 1.f)
	{
		SetActorTickEnabled(false);
	}
}

// Called to bind functionality to input 
void AJellyCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent *EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AJellyCharacterBase::Move);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AJellyCharacterBase::HandleJumpStarted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AJellyCharacterBase::HandleJumpEnded);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,this,&AJellyCharacterBase::Look);
		EnhancedInputComponent->BindAction(ThrowAction,ETriggerEvent::Started, this, &AJellyCharacterBase::StartPreparingThrow);
		EnhancedInputComponent->BindAction(ThrowAction,ETriggerEvent::Completed, this, &AJellyCharacterBase::ReleasePreparedThrow);
		EnhancedInputComponent->BindAction(ThrowAction,ETriggerEvent::Canceled, this, &AJellyCharacterBase::CancelPreparingThrow);
		EnhancedInputComponent->BindAction(MeleeAction,ETriggerEvent::Started,this, &AJellyCharacterBase::MeleeAttack);
		EnhancedInputComponent->BindAction(DashAction,ETriggerEvent::Started,this, &AJellyCharacterBase::Dash);

	}
	
}
// Input Actions Implementation
void AJellyCharacterBase::Move(const FInputActionValue& Value)
{
	if (!CanUseInput() || (DashComponent && DashComponent->IsDashing())) return;
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

void AJellyCharacterBase::HandleJumpStarted()
{
	if (CanUseInput())
	{
		Jump();
	}
}

void AJellyCharacterBase::HandleJumpEnded()
{
	StopJumping();
}

void AJellyCharacterBase::Dash()
{
	if (DashComponent)
	{
		DashComponent->TryDash();
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
	
	ToolToEquip->ApplyHeldState(this);
	
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

void AJellyCharacterBase::HandleThrowReady()
{
	if (!IsLocallyControlled() || !bIsPreparingThrow || !ThrowMontage) return;
	
	if (!CanUseInput() || !EquippedTool)
	{
		CancelPreparingThrow();
		return;
	}
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		AnimInstance->Montage_Pause(ThrowMontage);
	}
}

void AJellyCharacterBase::HandleThrowRelease()
{
	
	if (!IsLocallyControlled() || !bThrowReleasePending) return;
	
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	
	AEquippableToolBase* ToolToThrow = PendingThrowTool.Get();
	
	if (!CanUseInput() || !ToolToThrow || EquippedTool.Get() != ToolToThrow || !AnimInstance || !ThrowMontage)
	{
		CancelPreparingThrow();
		return;
	}
	
	if (!AnimInstance->Montage_IsActive(ThrowMontage))
	{
		CancelPreparingThrow();
		return;
	}
	const FVector ThrowDirection = PendingThrowDirection;
	
	bThrowReleasePending = false;
	PendingThrowTool.Reset();
	
	ServerThrow(ToolToThrow, ThrowDirection);
}

void AJellyCharacterBase::MeleeAttack()
{	
	if (!IsLocallyControlled() || !CanUseInput() || !CombatComponent || !StatusComponent || bIsPreparingThrow) return;
	if (!EquippedTool && !StatusComponent->IsChasing()) return;
	
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	
	if (!AnimInstance || !MeleeMontage) return;
	
	if (AnimInstance->Montage_IsActive(MeleeMontage)) return;
	
	if (ThrowMontage && AnimInstance->Montage_IsActive(ThrowMontage)) return;
	
	bMeleeHitPending = AnimInstance->Montage_Play(MeleeMontage) > 0.f;

	if (bMeleeHitPending)
	{
		ServerPlayMeleeMontage();
	}
}

void AJellyCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ApplyPlayerColor();
	ApplyRoleMovementSpeed();
}

void AJellyCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	ApplyPlayerColor();
	ApplyRoleMovementSpeed();
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

	const FLinearColor CurrentPlayerColor = PlayerColors[ColorIndex];
	
	if (!PlayerColorMaterial)
	{
		PlayerColorMaterial = GetMesh()->CreateDynamicMaterialInstance(PlayerColorMaterialIndex);
	}

	if (PlayerColorMaterial)
	{
		PlayerColorMaterial->SetVectorParameterValue(PlayerColorParameterName, CurrentPlayerColor);
	}
	if (PlayerRingDecal)
	{
		if (!PlayerRingMaterial)
		{
			PlayerRingMaterial = PlayerRingDecal->CreateDynamicMaterialInstance();
		}

		if (PlayerRingMaterial)
			{
				PlayerRingMaterial->SetVectorParameterValue(TEXT("RingColor"), CurrentPlayerColor);
				
			}
		PlayerRingDecal->SetVisibility(IsLocallyControlled(), true);
	}
		if (ThrowAimDecal)
		{
			if (!ThrowAimMaterial)
			{
				ThrowAimMaterial = ThrowAimDecal->CreateDynamicMaterialInstance();
			}
			if (ThrowAimMaterial)
			{
				ThrowAimMaterial->SetVectorParameterValue(TEXT("AimColor"), CurrentPlayerColor);
			}
		}
	}




bool AJellyCharacterBase::HasEquippedTool() const
{
	return IsValid(EquippedTool);
}

AEquippableToolBase* AJellyCharacterBase::GetEquippedTool() const
{
	return EquippedTool;
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

	if (!EquippedTool)
	{
		CancelPreparingThrow();
		return;
	}
	
	EquippedTool->ApplyHeldState(this);
	AddToolMappingContext(EquippedTool);
	
}

void AJellyCharacterBase::ServerThrow_Implementation(AEquippableToolBase* ExpectedTool, FVector_NetQuantizeNormal ThrowDirection)
{
	if (!IsValid(ExpectedTool) || EquippedTool.Get() != ExpectedTool) return;
	
	PerformThrow(ThrowDirection);
}

void AJellyCharacterBase::PerformThrow(const FVector& InThrowDirection)
{
	if (!HasAuthority() || !CanUseInput() || !EquippedTool || !EquippedTool->ToolMeshComponent) return;
	
	FVector ThrowDirection(InThrowDirection.X, InThrowDirection.Y,0.f);

	if (!ThrowDirection.Normalize()) return;
	
	AEquippableToolBase* ToolToThrow = EquippedTool;
	
	RemoveToolMappingContext(ToolToThrow);
	
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

bool AJellyCharacterBase::CanUseInput() const
{
	const UWorld *World = GetWorld();
	if (!World) return false;
	
	const AJellyGameStateBase* JellyGameState = World->GetGameState<AJellyGameStateBase>();

	if (!JellyGameState) return false;
	
	const bool bMatchIsPlaying = JellyGameState->GetMatchPhase() == EJellyMatchPhase::Playing;
	
	const bool bCharacterCanAct = !StatusComponent || !StatusComponent->bIsStunned;
	return bMatchIsPlaying && bCharacterCanAct;
}

void AJellyCharacterBase::ApplyRoleMovementSpeed()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	
	const AJellyPlayerState* JellyPlayerState = GetPlayerState<AJellyPlayerState>();
	
	if (!Movement || !JellyPlayerState) return;
	
	Movement->MaxWalkSpeed = JellyPlayerState->IsChaser() ? ChaserWalkSpeed : RunnerWalkSpeed;
	
}

void AJellyCharacterBase::StartPreparingThrow()
{
	if (!CanUseInput() || !EquippedTool || bIsPreparingThrow) return;
	
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

	if (!AnimInstance || !ThrowMontage) return;

	if (MeleeMontage && AnimInstance->Montage_IsActive(MeleeMontage)) return;
		
	if (AnimInstance->Montage_IsActive(ThrowMontage)) return;
	
	const float Duration = AnimInstance->Montage_Play(ThrowMontage);
	
	if (Duration <= 0.f) return;
	
	bThrowReleasePending = false;
	PendingThrowTool.Reset();
	bIsPreparingThrow = true;
	SetThrowAimVisible(true);
}

void AJellyCharacterBase::ReleasePreparedThrow()
{
	if (!bIsPreparingThrow) return;
	
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

	if (!CanUseInput() || !EquippedTool || !AnimInstance || !ThrowMontage)
	{
		CancelPreparingThrow();
		return;
	}

	if (!AnimInstance->Montage_IsActive(ThrowMontage))
	{
		CancelPreparingThrow();
		return;
	}
	PendingThrowDirection = GetActorForwardVector();
	PendingThrowDirection.Z = 0.f;

	if (!PendingThrowDirection.Normalize())
	{
		CancelPreparingThrow();
		return;
	}
	
	PendingThrowTool =EquippedTool.Get();
	bIsPreparingThrow = false;
	bThrowReleasePending = true;
	SetThrowAimVisible(false);
	
		AnimInstance->Montage_Resume(ThrowMontage);
	
}

void AJellyCharacterBase::CancelPreparingThrow()
{
	const bool bShouldStopMontage = bIsPreparingThrow || bThrowReleasePending;
	
	bIsPreparingThrow = false;
	bThrowReleasePending = false;
	PendingThrowTool.Reset();
	
	SetThrowAimVisible(false);

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	
	if (bShouldStopMontage && AnimInstance && ThrowMontage)
	{
		AnimInstance->Montage_Stop(0.1f, ThrowMontage);
	}
}

void AJellyCharacterBase::PawnClientRestart()
{
	Super::PawnClientRestart();
	
	if (PlayerRingDecal)
	{
		PlayerRingDecal->SetVisibility(true, true);
	}
	ApplyPlayerColor();
}

void AJellyCharacterBase::SetThrowAimVisible(bool bVisible)
{
	if (!ThrowAimDecal) return;
	
	const bool bShouldShow = bVisible && IsLocallyControlled();

	if (!bShouldShow)
	{
		ThrowAimDecal->SetVisibility(false,true);
		ThrowAnimAlpha = 0.f;
		SetActorTickEnabled(false);
		return;
	}
	ThrowAnimAlpha = 0.f;
	
	ThrowAimDecal->SetRelativeScale3D(FVector(1.f, 1.f, 0.12f));
	
	FVector ArrowLocation = ThrowAimDecal->GetRelativeLocation();
	ArrowLocation.X = 61.f;
	
	ThrowAimDecal->SetRelativeLocation(ArrowLocation);
	ThrowAimDecal->SetVisibility(true,true);
	SetActorTickEnabled(true);
}


void AJellyCharacterBase::HandleMeleeHit()
{
	if (!IsLocallyControlled() || !bMeleeHitPending) return;
	
	bMeleeHitPending = false;
	
	if (!CanUseInput() || !CombatComponent) return;
	
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	
	if (!AnimInstance || !MeleeMontage) return;
	
	if (!AnimInstance->Montage_IsActive(MeleeMontage)) return;
	
	CombatComponent->MeleeAttack();
}

void AJellyCharacterBase::ServerPlayMeleeMontage_Implementation()
{
	if (!CanUseInput() || !CombatComponent || !StatusComponent || !MeleeMontage) return;
	
	if (!EquippedTool && !StatusComponent->IsChasing()) return;

	MulticastPlayMeleeMontage();
}

void AJellyCharacterBase::MulticastPlayMeleeMontage_Implementation()
{
	if (IsLocallyControlled()) return;

	if (GetNetMode() == NM_DedicatedServer) return;
	
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	
	if (!AnimInstance || !MeleeMontage) return;
	
	AnimInstance->Montage_Play(MeleeMontage);
}
