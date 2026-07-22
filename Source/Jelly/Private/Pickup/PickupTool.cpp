// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/PickupTool.h"

APickupTool::APickupTool()
{
	
}



void APickupTool::InitializePickup()
{
	Super::InitializePickup();
	if (PickupMeshComponent)
	{
	
		PickupMeshComponent->SetSimulatePhysics(true);
		PickupMeshComponent->SetCollisionEnabled((ECollisionEnabled::QueryAndPhysics));
		PickupMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
		PickupMeshComponent->SetMassOverrideInKg(NAME_None,50.0f);
	}
}
