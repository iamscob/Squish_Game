
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JellyHUDWidget.generated.h"

class UTextBlock;
class UBorder;
class UWidget;
class UWidgetAnimation;


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
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidget> SB_ToolPanel;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ToolNameText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ToolHintText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidget> SB_TransferMessage;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TransferText;
	
	UPROPERTY(Transient, meta=(BindWidgetAnim))
	TObjectPtr<UWidgetAnimation>Anim_YouAreChaser;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidget> IMG_Vignette;
	
	UPROPERTY(Transient, meta=(BindWidgetAnim))
	TObjectPtr<UWidgetAnimation>Anim_Vignette;


	
private:
	
	void RefreshHUD();
	
	FTimerHandle HUDRefreshTimerHandle;
	
	bool bHasCachedChaserState = false;
	bool bWasChaser = false;
	
	FTimerHandle TransferMessageTimerHandle;
	
	void ShowTransferMessage();
	void HideTransferMessage();
};
