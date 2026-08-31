// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyGameStateBase.h"
#include "Net/UnrealNetwork.h"

EJellyMatchPhase AJellyGameStateBase::GetMatchPhase() const
{
	return MatchPhase;
}



int32 AJellyGameStateBase::GetRemainingTime() const
{
	return RemaningTime;
}

void AJellyGameStateBase::SetMatchPhase(EJellyMatchPhase NewPhase)
{
	if (!HasAuthority()) return;
	MatchPhase = NewPhase;
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
	
	DOREPLIFETIME(AJellyGameStateBase, MatchPhase);
	DOREPLIFETIME(AJellyGameStateBase, RemaningTime);
}

