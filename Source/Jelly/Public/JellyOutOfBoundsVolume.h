// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JellyOutOfBoundsVolume.generated.h"

class ATargetPoint;
class UBoxComponent;

UCLASS()
class JELLY_API AJellyOutOfBoundsVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJellyOutOfBoundsVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OutOfBounds")
	TObjectPtr<UBoxComponent> TriggerBox;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "OutOfBounds")
	TObjectPtr<ATargetPoint> ToolRespawnPoint;
	
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
