// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableToolBase.h"
#include "Character/JellyCharacterBase.h"
#include "InputMappingContext.h"
#include "JellyStatusComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AEquippableToolBase::AEquippableToolBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	
	ToolMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
	check(ToolMeshComponent!=nullptr);
	SetRootComponent(ToolMeshComponent);
	
	ToolMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ToolMeshComponent->SetGenerateOverlapEvents(false);
	
	ToolMeshComponent->SetNotifyRigidBodyCollision(true);
	ToolMeshComponent->OnComponentHit.AddDynamic(this, &AEquippableToolBase::OnToolHit);
	ToolMeshComponent->SetIsReplicated(true);
	
	PickupCollisionComponent = CreateDefaultSubobject<USphereComponent>("PickupCollision");
	PickupCollisionComponent->SetupAttachment(ToolMeshComponent);
	PickupCollisionComponent->SetSphereRadius(80.f);
	PickupCollisionComponent->SetGenerateOverlapEvents(false);
	PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void AEquippableToolBase::BeginPlay()
{
	Super::BeginPlay();

	if (!PickupCollisionComponent) return;
	if (HasAuthority())
	{
		PickupCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AEquippableToolBase::OnPickupBeginOverlap);
	}
	else
	{
		PickupCollisionComponent->SetGenerateOverlapEvents(false);
		PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Called every frame
void AEquippableToolBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEquippableToolBase::OnToolHit(
	UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& HitResult
	)
{
	if (!HasAuthority()) return;
	
	if (bHasProcessedHit || !Thrower || !OtherActor) return;
	
	AJellyCharacterBase* HitCharacter = Cast<AJellyCharacterBase>(OtherActor);
	
	if (!HitCharacter || HitCharacter == Thrower) return;
	
	UJellyStatusComponent* HitStatus = HitCharacter->FindComponentByClass<UJellyStatusComponent>();
	
	if (!HitStatus) return;
	
	FVector HitDirection = -NormalImpulse.GetSafeNormal();
	if (HitDirection.IsNearlyZero())
	{
		HitDirection = -HitResult.ImpactNormal.GetSafeNormal();
	}
	const bool bHitApplied = HitStatus->ApplyHit(Thrower, HitDirection, true);
	if (!bHitApplied) return;
	bHasProcessedHit = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,TEXT("Throw Hit"));
	StartPickupCooldown();
}

void AEquippableToolBase::MulticastPrepareForThrow_Implementation(FVector ThrowStart)
{
	if (!ToolMeshComponent) return;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(ThrowStart,false, nullptr,ETeleportType::TeleportPhysics);
	ToolMeshComponent->SetWorldScale3D(FVector(WorldScale));
	ToolMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	ToolMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ToolMeshComponent->SetSimulatePhysics(true);
	ToolMeshComponent->WakeAllRigidBodies();
}

void AEquippableToolBase::StartPickupCooldown()
{
	if (!HasAuthority() || !PickupCollisionComponent || !GetWorld()) return;
	
	bCanBePickedUp = false;
	PickupCollisionComponent->SetGenerateOverlapEvents(false);
	PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetWorldTimerManager().ClearTimer(PickupEnableTimerHandle);
	
	GetWorldTimerManager().SetTimer(PickupEnableTimerHandle, this,
		&AEquippableToolBase::EnablePickup,1.f,false);
}

void AEquippableToolBase::EnablePickup()
{
	if (!HasAuthority() || !PickupCollisionComponent || OwningCharacter) return;
	bCanBePickedUp = true;
	PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollisionComponent->SetGenerateOverlapEvents(true);
}

void AEquippableToolBase::ResetProcessedHit()
{
	bHasProcessedHit = false;
}

void AEquippableToolBase::OnPickupBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bCanBePickedUp || !OtherActor) return;
	
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(OtherActor);
	if (!Character || Character->HasEquippedTool()) return;
	if (Character->AttachExistingTool(this))
	{
		bCanBePickedUp = false;
		PickupCollisionComponent->SetGenerateOverlapEvents(false);
		PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AEquippableToolBase::MulticastPrepareForHeld_Implementation(AJellyCharacterBase* NewOwningCharacter)
{
	if (!NewOwningCharacter || !ToolMeshComponent ||!PickupCollisionComponent) return;
	
	PickupCollisionComponent->SetGenerateOverlapEvents(false);
	PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	ToolMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	
	ToolMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	
	ToolMeshComponent->SetSimulatePhysics(false);
	ToolMeshComponent->SetGenerateOverlapEvents(false);
	ToolMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	AttachToComponent(NewOwningCharacter->GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName(TEXT("RightHandIndex3")));
	SetActorRelativeScale3D(FVector(2.f));
}


