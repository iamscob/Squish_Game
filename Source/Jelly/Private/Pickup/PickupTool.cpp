// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/PickupTool.h"

APickupTool::APickupTool()
{
	
}



void APickupTool::InitializePickup()
{
	Super::InitializePickup();
	if (!PickupMeshComponent) return;
	
		PickupMeshComponent->SetSimulatePhysics(true);
		PickupMeshComponent->SetCollisionEnabled((ECollisionEnabled::QueryAndPhysics));
		PickupMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
		PickupMeshComponent->SetMassOverrideInKg(NAME_None,ToolMass);
	
}
