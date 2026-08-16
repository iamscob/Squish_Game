// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyGameStateBase.h"
#include "Net/UnrealNetwork.h"

EJellyRoundPhase AJellyGameStateBase::GetRoundPhase() const
{
	return RoundPhase;
}

int32 AJellyGameStateBase::GetCurrentRound() const
{
	return CurrentRound;
}

int32 AJellyGameStateBase::GetTotalRounds() const
{
	return TotalRounds;
}

int32 AJellyGameStateBase::GetRemainingTime() const
{
	return RemaningTime;
}

void AJellyGameStateBase::SetRoundPhase(EJellyRoundPhase NewPhase)
{
	if (!HasAuthority()) return;
	RoundPhase = NewPhase;
	ForceNetUpdate();
}

void AJellyGameStateBase::SetCurrentRound(int32 NewCurrentRound)
{
	if (!HasAuthority()) return;
	CurrentRound = FMath::Max(0, NewCurrentRound);
	ForceNetUpdate();
	
}

void AJellyGameStateBase::SetTotalRounds(int32 NewTotalRounds)
{
	if (!HasAuthority()) return;
	TotalRounds = FMath::Max(1, NewTotalRounds);
	ForceNetUpdate();
}

void AJellyGameStateBase::SetRemainingTime(int32 NewRemaningTime)
{
	if (!HasAuthority()) return;
	RemaningTime = FMath::Max(0, NewRemaningTime);
	ForceNetUpdate();
}

void AJellyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AJellyGameStateBase, RoundPhase);
	DOREPLIFETIME(AJellyGameStateBase, CurrentRound);
	DOREPLIFETIME(AJellyGameStateBase, TotalRounds);
	DOREPLIFETIME(AJellyGameStateBase, RemaningTime);
}

