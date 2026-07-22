// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup/PickupBase.h"
#include "PickupConsumable.generated.h"

/**
 * 
 */
UCLASS()
class JELLY_API APickupConsumable : public APickupBase
{
	GENERATED_BODY()
	
public:
	APickupConsumable();
	
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void InitializePickup() override;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Animation")
	float RotationSpeed = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Animation")
	float FloatingAmplitude = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Animation")
	float FloatingSpeed = 2.f;
	
protected:
	float AnimStartOffsetZ;
	
	



};
