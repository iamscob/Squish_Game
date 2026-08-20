// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyGameModeBase.h"

#include "JellyGameStateBase.h"
#include "JellyPlayerState.h"
#include "Character/JellyCharacterBase.h"

AJellyGameModeBase::AJellyGameModeBase()
{
	GameStateClass = AJellyGameStateBase::StaticClass();
	PlayerStateClass = AJellyPlayerState::StaticClass();
}

void AJellyGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	JellyGameStateBase = GetGameState<AJellyGameStateBase>();

	if (!JellyGameStateBase)
	{
		UE_LOG(LogTemp, Error, TEXT("JellyGameState is not created"));
		return;
	}
	JellyGameStateBase->SetRoundPhase(EJellyRoundPhase::Waiting);
	
	JellyGameStateBase->SetCurrentRound(1);
	JellyGameStateBase->SetTotalRounds(TotalRounds);
	JellyGameStateBase->SetRemainingTime(0);
	
	// GetWorldTimerManager().SetTimer(WaitingTimerHandle, this, &AJellyGameModeBase::StartCountdown,
	// 	1.f, false);
	
    TryStartJellyMatch();
}

void AJellyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	AJellyPlayerState* JellyPlayerState = NewPlayer 
	? NewPlayer->GetPlayerState<AJellyPlayerState>() 
	: nullptr;
	
	if (JellyPlayerState && JellyPlayerState->GetPlayerColorIndex() == 255)
	{
		const uint8 AvailableColorIndex = FindAvailableColorIndex();
		if (AvailableColorIndex < 6)
		{
			JellyPlayerState->SetPlayerColorIndex(AvailableColorIndex);
		}
	}
	
	TryStartJellyMatch();
}

void AJellyGameModeBase::StartCountdown()
{
	if (!JellyGameStateBase) return;
	
	CountdownTimeRemaining = FMath::Max(1, CountdownDuration);
	JellyGameStateBase->SetRoundPhase(EJellyRoundPhase::Countdown);
	JellyGameStateBase->SetRemainingTime(CountdownTimeRemaining);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow,
			FString::Printf(TEXT("%d"),CountdownTimeRemaining));
	}
	GetWorldTimerManager().SetTimer(PhaseTimerHandle,this, &AJellyGameModeBase::HandleCountdownTick,
		1.f, true);
}

void AJellyGameModeBase::HandleCountdownTick()
{
	if (!JellyGameStateBase) return;
	--CountdownTimeRemaining;
	JellyGameStateBase->SetRemainingTime(CountdownTimeRemaining);
	if (CountdownTimeRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
		StartRound();
		return;
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,1.f, FColor::Yellow,
			FString::Printf(TEXT("%d"),CountdownTimeRemaining));
	}
}

void AJellyGameModeBase::StartRound()
{
	if (!JellyGameStateBase) return;
	
	if (!SelectRandomChaser())
	{
		JellyGameStateBase->SetRoundPhase(EJellyRoundPhase::Waiting);
		return;
	}
	
	RoundTimeRemaining = FMath::Max(1, RoundDuration);
	JellyGameStateBase->SetRoundPhase(EJellyRoundPhase::Playing);
	JellyGameStateBase->SetRemainingTime(RoundTimeRemaining);
	ChaserPeriodStartTime = GetWorld()->GetTimeSeconds();
	bIsTrackingChaserTime = true;
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Green, TEXT("GO"));
	}
	GetWorldTimerManager().SetTimer(PhaseTimerHandle,this,
		&AJellyGameModeBase::HandleRoundTick,1.f, true);
}

void AJellyGameModeBase::HandleRoundTick()
{
	if (!JellyGameStateBase) return;
	--RoundTimeRemaining;
	JellyGameStateBase->SetRemainingTime(RoundTimeRemaining);
	if (RoundTimeRemaining <= 0) EndRound();
	
}

AJellyPlayerState* AJellyGameModeBase::FindCurrentChaser() const
{
	if (!JellyGameStateBase) return nullptr;
	for (APlayerState* PlayerState : JellyGameStateBase->PlayerArray)
	{
	AJellyPlayerState* JellyPlayerState = Cast<AJellyPlayerState>(PlayerState);	
	if (JellyPlayerState && JellyPlayerState->IsChaser()) return JellyPlayerState;
	}
	return nullptr;
}


void AJellyGameModeBase::EndRound()
{
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	if (!JellyGameStateBase) return;
	
	AJellyPlayerState* CurrentChaser = FindCurrentChaser();
	
	CommitChaserTime(CurrentChaser);
	bIsTrackingChaserTime = false;
	
	JellyGameStateBase->SetRemainingTime(0);
	JellyGameStateBase->SetRoundPhase(EJellyRoundPhase::Results);
	
	GetWorldTimerManager().SetTimer(ResultsTimerHandle,this, &AJellyGameModeBase::HandleResultsFinished,
		ResultsDuration,false);
}

bool AJellyGameModeBase::SelectRandomChaser()
{
	if (!JellyGameStateBase) return false;
TArray<AJellyPlayerState*> ValidPlayers;
	
	for (APlayerState* PlayerState : JellyGameStateBase->PlayerArray)
	{
		AJellyPlayerState* JellyPlayerState = Cast<AJellyPlayerState>(PlayerState);

		if (JellyPlayerState)
		{
			ValidPlayers.Add(JellyPlayerState);
		}
	}
if (ValidPlayers.IsEmpty())
{
	UE_LOG(LogTemp, Warning, TEXT("Cannot select chaser: no players"));
	return false;
}
	for (AJellyPlayerState* PLayerState: ValidPlayers)
	{
		PLayerState->SetIsChaser(false);
	}
	const int32 RandomIndex = FMath::RandRange(0, ValidPlayers.Num() - 1);
	
	AJellyPlayerState* SelectedChaser = ValidPlayers[RandomIndex];
	
	SelectedChaser->SetIsChaser(true);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("Chaser %s"),*SelectedChaser->GetPlayerName()));
	}
	return true;
}

bool AJellyGameModeBase::TryTransferChaser(AJellyCharacterBase* Attacker, AJellyCharacterBase* Target)
{
	if (!HasAuthority()) return false;
	
	if (!JellyGameStateBase || JellyGameStateBase->GetRoundPhase() != EJellyRoundPhase::Playing) return false;
	if (!Attacker || !Target||Attacker == Target) return false;
	AJellyPlayerState* AttackerPlayerState = Attacker->GetPlayerState<AJellyPlayerState>();
	AJellyPlayerState* TargetPlayerState = Target->GetPlayerState<AJellyPlayerState>();
	if (!AttackerPlayerState || ! TargetPlayerState) return false;
	if (!AttackerPlayerState->IsChaser()) return false;
	if (TargetPlayerState->IsChaser()) return false;
	
	CommitChaserTime(AttackerPlayerState);
	
	AttackerPlayerState->SetIsChaser(false);
	TargetPlayerState->SetIsChaser(true);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Orange, 
			FString::Printf(TEXT("Chaser Transfered: %s -> %s"),
				*AttackerPlayerState->GetPlayerName(),
				*TargetPlayerState->GetPlayerName())
				);
	}
	return true;
}

void AJellyGameModeBase::HandleResultsFinished()
{
	if (!JellyGameStateBase) return;
	
	const int32 CurrentRoundNumber = JellyGameStateBase->GetCurrentRound();
	const int32 TotalRoundCount= JellyGameStateBase->GetTotalRounds();

	if (CurrentRoundNumber>=TotalRoundCount)
	{
		FinishMatch();
		return;
	}
	JellyGameStateBase->SetCurrentRound(CurrentRoundNumber+1);	
StartCountdown();
}

void AJellyGameModeBase::FinishMatch()
{
	if (!JellyGameStateBase) return;
	JellyGameStateBase->SetRemainingTime(0);
	JellyGameStateBase->SetRoundPhase(EJellyRoundPhase::MatchFinished);
	
	float LowestChaserTime = TNumericLimits<float>::Max();
	TArray<AJellyPlayerState*> Winners;
	constexpr float TieToleranceSeconds = 0.05f;

	for (APlayerState* PlayerState : JellyGameStateBase->PlayerArray)
	{
		AJellyPlayerState* JellyPlayerState = Cast<AJellyPlayerState>(PlayerState);
		if (!JellyPlayerState) continue;
		const float PlayerChaserTime = JellyPlayerState->GetChaserTime();
		if (PlayerChaserTime < LowestChaserTime - TieToleranceSeconds)
		{
			LowestChaserTime = PlayerChaserTime;
			Winners.Reset();
			Winners.Add(JellyPlayerState);
		}

		else  if (FMath::IsNearlyEqual(PlayerChaserTime, LowestChaserTime,TieToleranceSeconds))
			{
			Winners.Add(JellyPlayerState);
		}
	}
	if (Winners.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,
			(TEXT("Match finished, no players found")));
		return;
	}
	FString ResultMessage;
	if (Winners.Num() == 1)
	{
		ResultMessage = FString::Printf(TEXT("Winner: %s (%.2f seconds as chaser)"),
			*Winners[0]->GetPlayerName(),
			LowestChaserTime);
	}
	else
	{
		TArray<FString> WinnerNames;
		for (const AJellyPlayerState* Winner:Winners)
		{
			WinnerNames.Add(Winner->GetPlayerName());
		}
		ResultMessage = TEXT("Draw");
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,
			ResultMessage);
			return;
		}
	}
}

void AJellyGameModeBase::CommitChaserTime(AJellyPlayerState* Chaser)
{
	if (!bIsTrackingChaserTime || !Chaser || !GetWorld())return;
	
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedTime = FMath::Max(0.f, CurrentTime - ChaserPeriodStartTime);
	
	Chaser->AddChaserTime(ElapsedTime);
	
	ChaserPeriodStartTime = CurrentTime;
}

void AJellyGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (!bMatchFlowStarted)
	{
		TryStartJellyMatch();
	}
}

void AJellyGameModeBase::TryStartJellyMatch()
{
	if (bMatchFlowStarted || !JellyGameStateBase) return;
	
	const int32 ConnectedPlayerCount = JellyGameStateBase->PlayerArray.Num();
	const int32 RequiredPlayerCount = FMath::Max(1, MinimumPlayers);

	if (ConnectedPlayerCount < RequiredPlayerCount)
	{
		JellyGameStateBase->SetRoundPhase(EJellyRoundPhase::Waiting);
		
		JellyGameStateBase->SetRemainingTime(0);
		return;
	}
	
	bMatchFlowStarted = true;
	GetWorldTimerManager().SetTimer(WaitingTimerHandle,this, &AJellyGameModeBase::StartRound,1.f,false);
}

uint8 AJellyGameModeBase::FindAvailableColorIndex() const
{
	constexpr uint8 ColorCount = 6;
	constexpr uint8 InvalidColorIndex = 255;
	
	bool UsedColors[ColorCount] =
		{
		false,
		false,
		false,
		false,
		false,
		false
	};
	if (!JellyGameStateBase) return InvalidColorIndex;
	for (APlayerState* PlayerState : JellyGameStateBase->PlayerArray)
	{
		const AJellyPlayerState* JellyPlayerState = Cast<AJellyPlayerState>(PlayerState);
		if (!JellyPlayerState) continue;
		const uint8 ColorIndex = JellyPlayerState->GetPlayerColorIndex();
		if (ColorIndex < ColorCount)
		{
			UsedColors[ColorIndex] = true;
		}
	}
	for (uint8 ColorIndex = 0; ColorIndex <ColorCount; ++ColorIndex)
	{
		if (!UsedColors[ColorIndex]) return ColorIndex;
	}
	return InvalidColorIndex;
}
