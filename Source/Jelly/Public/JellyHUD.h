// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "JellyHUD.generated.h"


class UJellyHUDWidget;

/**
 * 
 */
UCLASS()
class JELLY_API AJellyHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|HUD")
TSubclassOf<UJellyHUDWidget> HUDWidgetClass;
	
private:
	
	UPROPERTY()
	TObjectPtr<UJellyHUDWidget> HUDWidget;
};
