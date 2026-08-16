// Fill out your copyright notice in the Description page of Project Settings.

#include "JellyStatusComponent.h"
#include "Character/JellyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "JellyPlayerState.h"
#include "JellyGameModeBase.h"

// Sets default values for this component's properties
UJellyStatusComponent::UJellyStatusComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UJellyStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UJellyStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UJellyStatusComponent::SetIsChasing(bool bNewIsChasing)
{
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter) return;
	AJellyPlayerState* PlayerState = OwnerCharacter->GetPlayerState<AJellyPlayerState>();
	if (!PlayerState) return;
	PlayerState->SetIsChaser(bNewIsChasing);
	if (GEngine)
	{
		const FString StatusMessage = bNewIsChasing ? TEXT("Chasing")
			: TEXT("NotChasing");
		GEngine->AddOnScreenDebugMessage(-1, 3.f, bNewIsChasing ? FColor::Red
			:FColor::Green, StatusMessage);
	}
}

bool UJellyStatusComponent::ApplyHit(AJellyCharacterBase* Attacker, const FVector& HitDirection, bool bIsThrownHit)
	{
	
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter || !Attacker || bIsStunned) return false;
	
UJellyStatusComponent* AttackerStatus = Attacker->FindComponentByClass<UJellyStatusComponent>();
	if (!AttackerStatus) return false;
	
	UJellyStatusComponent* OwnerStatus = this;
	
	const bool bAttackerIsChaser = AttackerStatus->IsChasing();
	const bool bOwnerIsChaser = OwnerStatus->IsChasing();
	
		float StunDuration = 2.f;

		// If Chasing Player Attacks -> Running Player
		if (bAttackerIsChaser && !bOwnerIsChaser)
		{
		AJellyGameModeBase* JellyGameMode = GetWorld()->GetAuthGameMode<AJellyGameModeBase>();
			
			if (!JellyGameMode)return false;
			
			const bool bRoleTransfered = JellyGameMode->TryTransferChaser(Attacker,OwnerCharacter);
			
			if (!bRoleTransfered) return false;
			
		StunDuration = 2.5f;
		} // Running -> Chasing
		else if (!bAttackerIsChaser && bOwnerIsChaser)
		{
			StunDuration = 1.8f;
		} // Running -> Running
		else if (!bAttackerIsChaser && !bOwnerIsChaser)
		{
			StunDuration = 1.3f;
		} // Default
		else
		{
			StunDuration = 2.f;
		}
		bIsStunned = true;
	
		//Ragdoll 
	
		PreRagdollLocation = OwnerCharacter->GetActorLocation();
		PreRagdollRotation = OwnerCharacter->GetActorRotation();
	
		OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OwnerCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		OwnerCharacter->GetMesh()->SetSimulatePhysics(true);
		OwnerCharacter->GetMesh()->SetAllBodiesSimulatePhysics(true);
		OwnerCharacter->GetCharacterMovement()->DisableMovement();
	
	
		//Knockback
		FVector KnockbackDirection = HitDirection.GetSafeNormal();
		KnockbackDirection.Z = .8f;
	
		float KnockbackForce = bIsThrownHit ? 5000.f : 2800.f;
		OwnerCharacter->GetMesh()->AddImpulse(KnockbackDirection * KnockbackForce, NAME_None, true);
	
		OwnerCharacter->GetWorldTimerManager().SetTimer(
			StunTimerHandle,this,&UJellyStatusComponent::RecoverFromStun,
			StunDuration, false
			); 
	return true;
	}

void UJellyStatusComponent::RecoverFromStun()
{
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter) return;
	
	bIsStunned = false;
	
	OwnerCharacter->GetMesh()->SetSimulatePhysics(false);
	OwnerCharacter->GetMesh()->SetAllBodiesSimulatePhysics(false);
	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	OwnerCharacter->GetMesh()->AttachToComponent(
		OwnerCharacter->GetCapsuleComponent(),
		FAttachmentTransformRules::KeepWorldTransform
	);
	OwnerCharacter->GetCapsuleComponent()->SetWorldLocation(OwnerCharacter->GetMesh()->GetComponentLocation());
	
	OwnerCharacter->GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -100.f),
		FRotator(0.f, -90.f, 0.f)
	);

	OwnerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

bool UJellyStatusComponent::IsChasing() const
{
	const AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter) return false;
	const AJellyPlayerState* PlayerState = OwnerCharacter->GetPlayerState<AJellyPlayerState>();
	if (!PlayerState) return false;
	return PlayerState->IsChaser();
}