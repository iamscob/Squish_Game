// Fill out your copyright notice in the Description page of Project Settings.

#include "JellyPlayerState.h"

#include "NET/UnrealNetwork.h"

bool AJellyPlayerState::IsChaser() const
{
	return bIsChaser;
}

int32 AJellyPlayerState::GetPenaltyPoints() const
{
	return PenaltyPoints;
}

void AJellyPlayerState::SetIsChaser(bool bIsNewChaser)
{
	if (!HasAuthority()) return;
	++PenaltyPoints;
	ForceNetUpdate();
}

void AJellyPlayerState::ResetPenaltyPoints()
{
	if (!HasAuthority()) return;
	PenaltyPoints = 0;
	ForceNetUpdate();
}

void AJellyPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AJellyPlayerState, bIsChaser);
	DOREPLIFETIME(AJellyPlayerState, PenaltyPoints);
}
