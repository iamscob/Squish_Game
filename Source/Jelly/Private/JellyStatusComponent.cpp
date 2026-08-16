// Fill out your copyright notice in the Description page of Project Settings.

#include "JellyStatusComponent.h"
#include "Character/JellyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

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
	bIsChasing = bNewIsChasing;
	AJellyCharacterBase* OwnerCharater = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharater) return;
	if  (bIsChasing)
	{
		GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,TEXT("Chasing"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,TEXT("I'm NOT chasing"));		
	}
}

bool UJellyStatusComponent::ApplyHit(AJellyCharacterBase* Attacker, const FVector& HitDirection, bool bIsThrownHit)

	{
		AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
		if (!OwnerCharacter || !Attacker || bIsStunned) return false;
	UJellyStatusComponent* AttackerStatus = Attacker->FindComponentByClass<UJellyStatusComponent>();
		if (!AttackerStatus) return false;
		UJellyStatusComponent* OwnerStatus = this;
		
		float StunDuration = 2.f;

		// If Chasing Player Attacks -> Running Player
		if (AttackerStatus->IsChasing() && !OwnerStatus->IsChasing())
		{
			StunDuration = 2.5f; 
			AttackerStatus->SetIsChasing(false);
			OwnerStatus->SetIsChasing(true);
		
		} // Running -> Chasing
		else if (!AttackerStatus->IsChasing() && OwnerStatus->IsChasing())
		{
			StunDuration = 1.8f;
		} // Running -> Running
		else if (!AttackerStatus->IsChasing() && !OwnerStatus->IsChasing())
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
	return bIsChasing;
}