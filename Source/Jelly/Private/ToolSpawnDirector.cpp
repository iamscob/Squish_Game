// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSpawnDirector.h"

#include "JellyMapConfig.h"
#include "ToolSpawnPoint.h"
#include "EquippableToolBase.h"
#include "EngineUtils.h"

// Sets default values
AToolSpawnDirector::AToolSpawnDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

}

void AToolSpawnDirector::GatherSpawnPoints()
{
	SpawnPoints.Reset();
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	for (TActorIterator<AToolSpawnPoint> Iterator(World); Iterator; ++Iterator)
	{
		SpawnPoints.Add(*Iterator);
	}
	
}

void AToolSpawnDirector::ConfigureForPlayers(int32 PlayerCount)
{
	if (!HasAuthority() || !MapConfig) return;
	
	GatherSpawnPoints();

	if (SpawnPoints.IsEmpty()) return;
	
	TArray<TSubclassOf<AEquippableToolBase>> AvailableToolClasses;
	
	for (const TSubclassOf<AEquippableToolBase>& ToolClass : MapConfig->AllowedToolClasses)
	{
		if (ToolClass)
		{
			AvailableToolClasses.Add(ToolClass);
		}
	}

	if (AvailableToolClasses.IsEmpty()) return;
	
	for (AToolSpawnPoint* SpawnPoint: SpawnPoints)
	{
		if (SpawnPoint)
		{
			SpawnPoint->DeactivateSpawnPoint();
		}
	}
	
	for (int32 Index = SpawnPoints.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		SpawnPoints.Swap(Index, SwapIndex);
	}
	for (int32 Index = AvailableToolClasses.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		AvailableToolClasses.Swap(Index, SwapIndex);
	}
	const int32 RequestedToolCount = FMath::Max(1, PlayerCount/2);
		
	const int32 ActiveToolCount = FMath::Min(RequestedToolCount, SpawnPoints.Num());
		
	for (int32 Index = 0; Index < ActiveToolCount; ++Index)
	{
		AToolSpawnPoint* SpawnPoint = SpawnPoints[Index];
			
		if (!SpawnPoint) continue;
			
		const int32 ToolClassIndex = Index % AvailableToolClasses.Num();
			
		SpawnPoint->ActivateSpawnPoint(AvailableToolClasses[ToolClassIndex]);
	}
}
