// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/PickupConsumable.h"


APickupConsumable::APickupConsumable()
{
	
}

void APickupConsumable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FRotator CurrentRotation = PickupMeshComponent->GetRelativeRotation();
	CurrentRotation.Yaw += RotationSpeed * DeltaTime;
	PickupMeshComponent->SetRelativeRotation(CurrentRotation);
	
	float Time = GetWorld()->GetTimeSeconds();
	float FloatingOffset = FMath::Sin(Time * FloatingSpeed) * FloatingAmplitude;
		
	FVector NewLocation = GetActorLocation();
	NewLocation.Z = AnimStartOffsetZ + FloatingOffset;
	SetActorLocation(NewLocation);
}

void APickupConsumable::InitializePickup()
{
	Super::InitializePickup();
	if (PickupMeshComponent)
	{
		
	
		PickupMeshComponent->SetSimulatePhysics(false);
		PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		PickupMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
		AnimStartOffsetZ = GetActorLocation().Z;
	}
}
