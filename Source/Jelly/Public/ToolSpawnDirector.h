// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ToolSpawnDirector.generated.h"

class UJellyMapConfig;
class AToolSpawnPoint;

UCLASS()
class JELLY_API AToolSpawnDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AToolSpawnDirector();
	
	void ConfigureForPlayers(int32 PlayerCount	);

protected:
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jelly|Tools")
TObjectPtr<UJellyMapConfig> MapConfig;
	
	
private:

	void GatherSpawnPoints();
	
	UPROPERTY()
	TArray<TObjectPtr<AToolSpawnPoint>> SpawnPoints;
	
};
