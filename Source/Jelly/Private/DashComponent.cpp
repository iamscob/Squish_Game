// Fill out your copyright notice in the Description page of Project Settings.


#include "DashComponent.h"

#include "JellyPlayerState.h"
#include "Character/JellyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values for this component's properties
UDashComponent::UDashComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// ...
}


// Called when the game starts
void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UDashComponent::TryDash()
{
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(GetOwner());
	
	if (!Character || !CanDash()) return;
	
	FVector DashDirection = Character->GetActorForwardVector();
	DashDirection.Z = 0.f;
	DashDirection.Normalize();

	if (Character->HasAuthority())
	{
		StartDash(DashDirection);
		return;
	}
	ServerTryDash(DashDirection);
}

bool UDashComponent::CanDash() const
{
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(GetOwner());
	
	const UWorld* World = GetWorld();

	if (!Character || !World || bIsDashing || !Character->CanUseInput()) return false;
	
	const AJellyPlayerState* PlayerState = Character->GetPlayerState<AJellyPlayerState>();
	
	if (!PlayerState || !PlayerState->IsChaser()) return false;
	
	return World->GetTimeSeconds() >= NextDashTime;
}

void UDashComponent::ServerTryDash_Implementation(FVector_NetQuantizeNormal RequestedDirection)
{
	StartDash(RequestedDirection);
}

void UDashComponent::StartDash(const FVector& RequestedDirection)
{
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(GetOwner());
	
	const UWorld* World = GetWorld();
	
	if (!Character || !Character->HasAuthority() || !World || !CanDash()) return;
	
	FVector SafeDirection(RequestedDirection.X, RequestedDirection.Y, 0.f);
	
	if (!SafeDirection.Normalize())
	{
		SafeDirection = Character->GetActorForwardVector();
		
		SafeDirection.Z = 0.f;
		SafeDirection.Normalize();
	}
	
	MulticastStartDash(SafeDirection);
	World->GetTimerManager().ClearTimer(DashTimerHandle);
	World->GetTimerManager().SetTimer(DashTimerHandle, this, &UDashComponent::EndDash, DashDuration, false);
	Character->ForceNetUpdate();
}

void UDashComponent::EndDash()
{
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(GetOwner());
	
	if (!Character || !Character->HasAuthority()) return;
	
	MulticastEndDash();
	Character->ForceNetUpdate();
}

void UDashComponent::MulticastStartDash_Implementation(FVector_NetQuantizeNormal DashDirection)
{
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(GetOwner());
	if (!Character) return;
	
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	
	if (!Movement) return;
	
	bIsDashing = true;
	
	SavedGroundFriction = Movement->GroundFriction;
	
	SavedBrakingDeceleration = Movement->BrakingDecelerationWalking;
	
	Movement->GroundFriction = 0.f;
	Movement->BrakingDecelerationWalking = 0.f;
	
	const float SafeDuration = FMath::Max(DashDuration, .01f);
	
	const float DashSpeed = DashDistance/SafeDuration;
	
	Movement->MaxWalkSpeed = DashSpeed;
	
	Movement->Velocity = FVector(DashDirection) * DashSpeed;
	
}

void UDashComponent::MulticastEndDash_Implementation()
{
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(GetOwner());

	if (!Character) return;
	
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();

	if (Movement && bIsDashing)
	{
		Movement->GroundFriction = SavedGroundFriction;
		Movement->BrakingDecelerationWalking = SavedBrakingDeceleration;
	}
	bIsDashing = false;

	if (UWorld* World = GetWorld())
	{
		NextDashTime = World->GetTimeSeconds() + FMath::Max(0.f, DashCooldown);
	}
	Character->ApplyRoleMovementSpeed();
}

bool UDashComponent::IsDashing() const
{
	return bIsDashing;
}
