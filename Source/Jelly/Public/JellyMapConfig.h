// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "JellyMapConfig.generated.h"

class AEquippableToolBase;


UCLASS(BlueprintType)
class JELLY_API UJellyMapConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Jelly|Tools")
	TArray<TSubclassOf<AEquippableToolBase>> AllowedToolClasses;
	
};
