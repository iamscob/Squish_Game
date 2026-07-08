// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "EquippableToolDefinition.generated.h"

class AEquippableToolBase;
class UInputMappingContext;

UCLASS(BlueprintType,Blueprintable)
class JELLY_API UEquippableToolDefinition : public UItemDefinition
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<AEquippableToolBase> ToolAsset;
	
	virtual UEquippableToolDefinition* CreateItemCopy(UObject* Outer) const override;
};


