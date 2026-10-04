// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ToolSpawnPoint.generated.h"

class AEquippableToolBase;
class USceneComponent;

UCLASS()
class JELLY_API AToolSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AToolSpawnPoint();
	
	void ActivateSpawnPoint(TSubclassOf<AEquippableToolBase> InToolClass);
	void DeactivateSpawnPoint();
	
	bool IsSpawnPointActive() const;
	
	AEquippableToolBase* GetSpawnedTool() const;


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Spawn")
	TObjectPtr<USceneComponent> SceneRoot;
	
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tool Spawn")
	TObjectPtr<AEquippableToolBase> SpawnedTool;
	
private:
	void SpawnTool(TSubclassOf<AEquippableToolBase> InToolClass);
	
	bool bIsActive = false;
	
};
