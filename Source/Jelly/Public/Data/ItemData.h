// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"

class UItemDefinition;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None		UMETA(DisplayName = "None"),
	Tool		UMETA(DisplayName = "Tool"),
	Consumable	UMETA(DisplayName = "Consumable")
};


USTRUCT(BlueprintType)
struct FItemText
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	
	FText Description;
};

USTRUCT(BlueprintType) 
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	EItemType ItemType = EItemType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	FText ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	FItemText ItemText;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemData")
	TSoftObjectPtr<UItemDefinition> ItemBase;
	
};

