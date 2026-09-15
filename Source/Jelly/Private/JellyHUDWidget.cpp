// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "JellyPlayerState.h"
#include "JellyGameStateBase.h"
#include "TimerManager.h"
#include  "EquippableToolBase.h"
#include "Character/JellyCharacterBase.h"

void UJellyHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SB_TransferMessage)
	{
		SB_TransferMessage->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (IMG_Vignette)
	{
		IMG_Vignette->SetVisibility(ESlateVisibility::HitTestInvisible);
		IMG_Vignette->SetRenderOpacity(0.f);
	}
	{
		
	}
	
	RefreshHUD();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(HUDRefreshTimerHandle,this, &UJellyHUDWidget::RefreshHUD,.2f,true);
	}
	
	
}

void UJellyHUDWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HUDRefreshTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(TransferMessageTimerHandle);
	}
	
	Super::NativeDestruct();
}

void UJellyHUDWidget::RefreshHUD()
{
	UWorld* World = GetWorld();
	if (!World || !TimerText) return;
	
	const AJellyGameStateBase* JellyGameState = World->GetGameState<AJellyGameStateBase>();

	if (!JellyGameState) return;
	
	const int32 RemainingSeconds = FMath::Max(0, JellyGameState->GetRemainingTime());
	const int32 Minutes = RemainingSeconds/60;
	const int32 Seconds = RemainingSeconds % 60;
	
	TimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds)));
	
	const EJellyMatchPhase MatchPhase = JellyGameState->GetMatchPhase();

	if (SB_PhaseMessage && PhaseText)
	{
		switch (MatchPhase)
		{
		case EJellyMatchPhase::Waiting:
			{
				SB_PhaseMessage->SetVisibility(ESlateVisibility::HitTestInvisible);
				PhaseText->SetText(FText::FromString(TEXT("Waiting For Players!")));
				break;
			}
		case EJellyMatchPhase::Countdown:
			{
				SB_PhaseMessage->SetVisibility(ESlateVisibility::HitTestInvisible);
				PhaseText->SetText(FText::AsNumber(RemainingSeconds));
				break;
			}
		case EJellyMatchPhase::Playing:
			{
				SB_PhaseMessage->SetVisibility(ESlateVisibility::Collapsed);
				break;
			}
		case EJellyMatchPhase::Results:
			{
				SB_PhaseMessage->SetVisibility(ESlateVisibility::Collapsed);
				break;
			}
			default:
			{
				SB_PhaseMessage->SetVisibility(ESlateVisibility::Collapsed);
				break;
			}
		}
		
	}
	
	if (SB_Results && ResultsTitleText && ResultsText)
	{
		if (MatchPhase != EJellyMatchPhase::Results)
		{
			SB_Results->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			SB_Results->SetVisibility(ESlateVisibility::HitTestInvisible);
			TArray<AJellyPlayerState*> SortedPlayers;
			for (APlayerState* PlayerState : JellyGameState->PlayerArray)
			{
				AJellyPlayerState* JellyPlayerState = Cast<AJellyPlayerState>(PlayerState);
				
				if (JellyPlayerState)
				{
					SortedPlayers.Add(JellyPlayerState);
				}
			}
			SortedPlayers.Sort([] (const AJellyPlayerState& A, const AJellyPlayerState& B)
			{
				return A.GetChaserTime() < B.GetChaserTime();
			});
			if (SortedPlayers.IsEmpty())
			{
				ResultsTitleText->SetText(FText::FromString(TEXT("NO PLAYERS")));
				ResultsText->SetText(FText::GetEmpty());
			}
			else
			{
				constexpr float TieTolerance = .05f;
				
				const float BestTime = SortedPlayers[0]->GetChaserTime();
				
				int32 WinnerCount = 0;
				
				for (const AJellyPlayerState* Player : SortedPlayers)
				{
					if (FMath::IsNearlyEqual(Player->GetChaserTime(),BestTime,TieTolerance))
					{
						++WinnerCount;
					}
				}
				if (WinnerCount > 1)
				{
					ResultsTitleText->SetText(FText::FromString(TEXT("DRAW")));
				}
				else
				{
					ResultsTitleText->SetText(FText::FromString(FString::Printf(TEXT("WINNER: %s"),
						*SortedPlayers[0]->GetPlayerName())));
				}
				FString ResultLines;
				
				for (int32 Index = 0; Index < SortedPlayers.Num(); ++Index)
				{
					const AJellyPlayerState* Player = SortedPlayers[Index];
					const float PlayerTime = Player->GetChaserTime();
					const bool bIsWinner = FMath::IsNearlyEqual(PlayerTime,BestTime, TieTolerance);
					
					ResultLines += FString::Printf(TEXT("%d. %s - %.2f s%s\n"),Index + 1,
						*Player->GetPlayerName(),PlayerTime,bIsWinner ? TEXT("WINNER"):TEXT(""));
				}
				ResultsText->SetText(FText::FromString(ResultLines));
			}	
		}
	}

	if (SB_ToolPanel && ToolNameText && ToolHintText)
	{
		APlayerController* ToolPlayerController = GetOwningPlayer();
		
		AJellyCharacterBase* LocalCharacter = ToolPlayerController 
		? Cast<AJellyCharacterBase>(ToolPlayerController->GetPawn()) : nullptr;
	
		AEquippableToolBase* CurrentTool = LocalCharacter ? LocalCharacter->GetEquippedTool() : nullptr;
		
		SB_ToolPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
		
		if (CurrentTool)
		{
			const FText DisplayName = CurrentTool->ToolDisplayName;
			ToolNameText->SetText(DisplayName);
			ToolHintText->SetText(FText::FromString(TEXT("CTRL")));
		}
		else
		{
			ToolNameText->SetText(FText::FromString(TEXT("EMPTY")));
			ToolHintText->SetText(FText::FromString(TEXT("NO TOOL")));
		}
	
	}
	
	if (!SB_RolePanel || !RoleText || !Border_RoleBG) return;
	
	const bool bShouldShowRole = MatchPhase == EJellyMatchPhase::Playing;
	
	APlayerController* PlayerController = GetOwningPlayer();
	
	const AJellyPlayerState* JellyPlayerState = PlayerController
	? PlayerController->GetPlayerState<AJellyPlayerState>() : nullptr;
	
	if (JellyPlayerState)
	{
		const bool bIsChaserNow = JellyPlayerState->IsChaser();

		if (!bHasCachedChaserState)
		{
			bHasCachedChaserState = true;
			bWasChaser = bIsChaserNow;

			if (bIsChaserNow)
			{
				ShowTransferMessage();
			}
		}
		else
		{
			if (!bWasChaser && bIsChaserNow)
			{
				ShowTransferMessage();
			}
			bWasChaser = bIsChaserNow;
		}
	}

	if (!bShouldShowRole || !JellyPlayerState)
	{
		SB_RolePanel->SetVisibility(ESlateVisibility::Collapsed);
		return;
	} 
	
	SB_RolePanel->SetVisibility(ESlateVisibility::HitTestInvisible);
	
	const bool bIsChaser = JellyPlayerState->IsChaser();
	
	RoleText->SetText(FText::FromString(bIsChaser ? TEXT("CHASER") : TEXT("RUNNER")));
	
	const FLinearColor RoleColor = bIsChaser ? FLinearColor::Red 
	: FLinearColor::Green;
	Border_RoleBG->SetBrushColor(RoleColor);
}

void UJellyHUDWidget::ShowTransferMessage()
{
	UWorld* World = GetWorld();
	if (!World || !SB_TransferMessage) return;

	SB_TransferMessage->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (Anim_YouAreChaser)
	{
		StopAnimation(Anim_YouAreChaser);
		PlayAnimation(Anim_YouAreChaser);
	}
		

	
	if (IMG_Vignette && Anim_Vignette)
	{
		IMG_Vignette->SetVisibility(ESlateVisibility::HitTestInvisible);
		StopAnimation(Anim_Vignette);
		PlayAnimation(Anim_Vignette);
	}
	
	World->GetTimerManager().ClearTimer(TransferMessageTimerHandle);
	World->GetTimerManager().SetTimer(TransferMessageTimerHandle,this, &UJellyHUDWidget::HideTransferMessage,1.2f, false);
	
}

void UJellyHUDWidget::HideTransferMessage()
{
	if (SB_TransferMessage)
	{
		SB_TransferMessage->SetVisibility(ESlateVisibility::Collapsed);
	}
}
