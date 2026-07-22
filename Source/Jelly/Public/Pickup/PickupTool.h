// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup/PickupBase.h"
#include "PickupTool.generated.h"

/**
 * 
 */
UCLASS()
class JELLY_API APickupTool : public APickupBase
{
	GENERATED_BODY()
	
public: 
	APickupTool();
	
	virtual void InitializePickup() override;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Tool|Physics")
	float ToolMass = 50.f;
};
