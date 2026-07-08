// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/ItemData.h"
#include "ItemDefinition.generated.h"


UCLASS(BlueprintType, Blueprintable)
class JELLY_API UItemDefinition : public UDataAsset
{
	GENERATED_BODY()
	
	
public: 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	FText ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	EItemType ItemType; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	FItemText ItemText;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	TSoftObjectPtr<UStaticMesh> WorldMesh;
	
	virtual UItemDefinition* CreateItemCopy(UObject* Outer) const;
};
