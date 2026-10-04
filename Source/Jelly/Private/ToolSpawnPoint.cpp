// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSpawnPoint.h"
#include "EquippableToolBase.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

// Sets default values
AToolSpawnPoint::AToolSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

}

void AToolSpawnPoint::ActivateSpawnPoint(TSubclassOf<AEquippableToolBase> InToolClass)
{
	if (!HasAuthority() || !InToolClass) return;
	
	bIsActive  = true;
	SpawnTool(InToolClass);
}


void AToolSpawnPoint::DeactivateSpawnPoint()
{
	if (!HasAuthority()) return;
	
	bIsActive = false;
}

bool AToolSpawnPoint::IsSpawnPointActive() const
{
	return bIsActive;
}

AEquippableToolBase* AToolSpawnPoint::GetSpawnedTool() const
{
	return SpawnedTool;
}

void AToolSpawnPoint::SpawnTool(TSubclassOf<AEquippableToolBase> InToolClass)
{
	if (!HasAuthority() || !bIsActive || IsValid(SpawnedTool.Get())|| !InToolClass || !GetWorld()) return;
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	SpawnedTool = GetWorld()->SpawnActor<AEquippableToolBase>(InToolClass, GetActorTransform(), SpawnParameters);
	
	if (SpawnedTool)
	{
		SpawnedTool->InitializeWorldTool(GetActorTransform());
	}
}


