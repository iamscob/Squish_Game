// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "JellyPlayerState.h"
#include "JellyGameStateBase.h"
#include "TimerManager.h"

void UJellyHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
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
	
	if (!SB_RolePanel || !RoleText || !Border_RoleBG) return;
	
	const bool bShouldShowRole = MatchPhase == EJellyMatchPhase::Playing;
	
	APlayerController* PlayerController = GetOwningPlayer();
	
	const AJellyPlayerState* JellyPlayerState = PlayerController
	? PlayerController->GetPlayerState<AJellyPlayerState>() : nullptr;

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
