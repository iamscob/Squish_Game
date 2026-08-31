// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyHUD.h"

#include "JellyHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void AJellyHUD::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PlayerController = GetOwningPlayerController();

	if (!PlayerController || !PlayerController->IsLocalController() || !HUDWidgetClass) return;
	
	
	HUDWidget = CreateWidget<UJellyHUDWidget>(PlayerController,HUDWidgetClass);

	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}
}
