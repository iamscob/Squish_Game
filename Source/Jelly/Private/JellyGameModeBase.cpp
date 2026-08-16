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
	
	GetWorldTimerManager().SetTimer(WaitingTimerHandle, this, &AJellyGameModeBase::StartCountdown,
		1.f, false);
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

void AJellyGameModeBase::EndRound()
{
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	if (!JellyGameStateBase) return;
	{
		JellyGameStateBase->SetRemainingTime(0);
		JellyGameStateBase->SetRoundPhase(EJellyRoundPhase::Results);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1,5.f,FColor::Red, TEXT("Round Finished"));
		}
	}
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
	if (!TargetPlayerState->IsChaser()) return false;
	
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
