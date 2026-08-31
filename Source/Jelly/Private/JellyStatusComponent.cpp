// Fill out your copyright notice in the Description page of Project Settings.

#include "JellyStatusComponent.h"
#include "Character/JellyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "JellyPlayerState.h"
#include "JellyGameModeBase.h"
#include "Net\UnrealNetwork.h"
#include "JellyGameStateBase.h"

// Sets default values for this component's properties
UJellyStatusComponent::UJellyStatusComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

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

bool UJellyStatusComponent::ApplyHit(AJellyCharacterBase* Attacker, const FVector& HitDirection, EJellyHitType HitType)
	{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
		if (!OwnerActor || !OwnerActor->HasAuthority() || !World) return false;
	
	const AJellyGameStateBase* JellyGameState = World->GetGameState<AJellyGameStateBase>();
	
	if (!JellyGameState || JellyGameState->GetMatchPhase() != EJellyMatchPhase::Playing) return false;
	
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	
	if (!OwnerCharacter || !Attacker || Attacker == OwnerCharacter||bIsStunned) return false;
	
	UJellyStatusComponent* AttackerStatus = Attacker->FindComponentByClass<UJellyStatusComponent>();
	
	if (!AttackerStatus) return false;
	
	const bool bAttackerIsChaser = AttackerStatus->IsChasing();
	
	const bool bOwnerIsChaser = IsChasing();

	if (HitType == EJellyHitType::HandMelee && !bAttackerIsChaser) return false;
		
	if (bAttackerIsChaser && !bOwnerIsChaser)
	{
		AJellyGameModeBase* JellyGameMode = World->GetAuthGameMode<AJellyGameModeBase>();

		if (!JellyGameMode || !JellyGameMode->TryTransferChaser(Attacker, OwnerCharacter)) return false;
	}
	
	float StunDuration = 1.f;
	float HorizontalSpeed = 2000.f;
	float VerticalSpeed = 400.f;

	switch (HitType)
	{
	case EJellyHitType::HandMelee:
		{
			StunDuration = .8f;
			HorizontalSpeed = 1800.f;
			VerticalSpeed = 350.f;
			break;
		}
	case EJellyHitType::ToolMelee:
		{
			StunDuration = 1.6f;
			HorizontalSpeed = 2500.f;
			VerticalSpeed = 500.f;
			break;
		}
	case EJellyHitType::ThrownTool:
		{
			StunDuration = 1.6f;
			HorizontalSpeed = 3500.f;
			VerticalSpeed = 750.f;
			break;
		}
		default: return false;
	}
		bIsStunned = true;
	
		const FVector HorizontalDirection = FVector(HitDirection.X, HitDirection.Y, 0.f).GetSafeNormal();
		const FVector LaunchVelocity = HorizontalDirection * HorizontalSpeed + FVector::UpVector * VerticalSpeed;
	
		MulticastStartRagdoll(LaunchVelocity);
	
	OwnerCharacter->GetWorldTimerManager().ClearTimer(StunTimerHandle);
	OwnerCharacter->GetWorldTimerManager().SetTimer(StunTimerHandle, this,
		&UJellyStatusComponent::RecoverFromStun, StunDuration, false);
	
	return true;
	}

void UJellyStatusComponent::RecoverFromStun()
{
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !GetWorld() || !OwnerCharacter->GetMesh()||
		!OwnerCharacter->GetCapsuleComponent()) return;
	
	USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();
	UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent();
	
	const FVector RagdollCenter = CharacterMesh->Bounds.Origin;
	const FVector TraceStart = RagdollCenter + FVector(0.f,0.f, 100.f);
	const FVector TraceEnd = RagdollCenter - FVector(0.f,0.f, 500.f);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	
	FHitResult GroundHit;
	
	const bool bFoundGround = GetWorld()->LineTraceSingleByChannel(GroundHit,
		TraceStart, TraceEnd,ECC_Visibility,QueryParams);
	
	FVector SafeCapsuleLocation = OwnerCharacter->GetActorLocation();
	if (bFoundGround)
	{
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		
		SafeCapsuleLocation = GroundHit.ImpactPoint + FVector::UpVector * (CapsuleHalfHeight +5.f);
	}
	
	bIsStunned = false;
	
	MulticastRecoverFromStun(SafeCapsuleLocation);
	
}

bool UJellyStatusComponent::IsChasing() const
{
	const AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter) return false;
	const AJellyPlayerState* PlayerState = OwnerCharacter->GetPlayerState<AJellyPlayerState>();
	if (!PlayerState) return false;
	return PlayerState->IsChaser();
}

void UJellyStatusComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UJellyStatusComponent, bIsStunned);
}

void UJellyStatusComponent::MulticastStartRagdoll_Implementation(FVector LaunchVelocity)
{
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter || !GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(RagdollBlendTimerHandle);
	
	PreRagdollLocation = OwnerCharacter->GetActorLocation();
	PreRagdollRotation = OwnerCharacter->GetActorRotation();
	
	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OwnerCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OwnerCharacter->GetMesh()->SetSimulatePhysics(true);
	OwnerCharacter->GetMesh()->SetPhysicsBlendWeight(1.f);
	OwnerCharacter->GetMesh()->SetAllBodiesSimulatePhysics(true);
	OwnerCharacter->GetCharacterMovement()->DisableMovement();
	OwnerCharacter->GetMesh()->AddImpulse(LaunchVelocity, NAME_None,true);
}

void UJellyStatusComponent::MulticastRecoverFromStun_Implementation(FVector RecoveryLocation)
{
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter) return;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RagdollBlendTimerHandle);
	}
	
	OwnerCharacter->GetMesh()->SetPhysicsBlendWeight(0.f);
	OwnerCharacter->GetMesh()->SetAllBodiesSimulatePhysics(false);
	OwnerCharacter->GetMesh()->SetSimulatePhysics(false);
	OwnerCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OwnerCharacter->GetCapsuleComponent()->SetWorldLocation(RecoveryLocation,false,nullptr,ETeleportType::TeleportPhysics);
	OwnerCharacter->GetMesh()->AttachToComponent(OwnerCharacter->GetCapsuleComponent(),FAttachmentTransformRules::KeepWorldTransform);
	OwnerCharacter->GetMesh()->SetRelativeLocationAndRotation(
			FVector(0.f, 0.f, -100.f),
			FRotator(0.f, -90.f, 0.f)
		);
	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OwnerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void UJellyStatusComponent::ResetForNewMatch(const FVector& SpawnLocation)
{
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority()) return;;
	
	OwnerCharacter->GetWorldTimerManager().ClearTimer(StunTimerHandle);
	
	bIsStunned = false;
	
	MulticastRecoverFromStun_Implementation(SpawnLocation);
	
	OwnerCharacter->ForceNetUpdate();
}


