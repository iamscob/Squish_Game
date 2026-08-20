// Fill out your copyright notice in the Description page of Project Settings.

#include "JellyPlayerState.h"
#include "Character\JellyCharacterBase.h"
#include "NET/UnrealNetwork.h"

bool AJellyPlayerState::IsChaser() const
{
	return bIsChaser;
}

int32 AJellyPlayerState::GetChaserTime() const
{
	return ChaserTimeSeconds;
}

void AJellyPlayerState::SetIsChaser(bool bIsNewChaser)
{
	if (!HasAuthority()) return;
	if (bIsChaser == bIsNewChaser) return;
	
	bIsChaser = bIsNewChaser;
	ForceNetUpdate();
}

void AJellyPlayerState::AddChaserTime(float Seconds)
{
	if (!HasAuthority() || Seconds <= 0.f) return;
	ChaserTimeSeconds += Seconds;
	ForceNetUpdate();
}

void AJellyPlayerState::ResetChaserTime()
{
	if (!HasAuthority()) return;
	ChaserTimeSeconds = 0.f;
	ForceNetUpdate();
}

uint8 AJellyPlayerState::GetPlayerColorIndex() const
{
	return PlayerColorIndex;
}


void AJellyPlayerState::SetPlayerColorIndex(uint8 NewColorIndex)
{
	if (!HasAuthority() || NewColorIndex > 5) return;
	PlayerColorIndex = NewColorIndex;
	
	OnRep_PlayerColorIndex();
	
	ForceNetUpdate();
}

void AJellyPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AJellyPlayerState, bIsChaser);
	DOREPLIFETIME(AJellyPlayerState, ChaserTimeSeconds);
	DOREPLIFETIME(AJellyPlayerState, PlayerColorIndex);
	
}

void AJellyPlayerState::OnRep_PlayerColorIndex()
{
	AJellyCharacterBase* JellyCharacter = Cast<AJellyCharacterBase>(GetPawn());
	if (JellyCharacter)
	{
		JellyCharacter->ApplyPlayerColor();
	}
}


