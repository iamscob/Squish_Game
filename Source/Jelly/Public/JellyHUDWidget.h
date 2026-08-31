// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JellyHUDWidget.generated.h"

class UTextBlock;
class UBorder;
class UWidget;

UCLASS()
class JELLY_API UJellyHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TimerText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidget> SB_RolePanel;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> Border_RoleBG;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> RoleText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidget> SB_PhaseMessage;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PhaseText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidget> SB_Results;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ResultsTitleText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ResultsText;
	
private:
	
	void RefreshHUD();
	
	FTimerHandle HUDRefreshTimerHandle;
};
