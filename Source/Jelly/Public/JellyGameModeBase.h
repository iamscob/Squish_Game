// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JellyGameModeBase.generated.h"

class AJellyGameStateBase;
class AJellyCharacterBase;
class AJellyPlayerState;
class APlayerController;
class AController;

UCLASS()
class JELLY_API AJellyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	AJellyGameModeBase();
	bool TryTransferChaser(AJellyCharacterBase* Attacker, AJellyCharacterBase* Target);

protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Round")
	int32 CountdownDuration = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Round")
	int32 RoundDuration = 10;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Round")
	int32 TotalRounds = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Round")
	float ResultsDuration = 3.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Round")
	int32 MinimumPlayers = 1;
	
	
	virtual void  PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	
private:
	
	void StartCountdown();
	
	void HandleCountdownTick();
	
	void StartRound();
	
	void HandleRoundTick();
	
	void EndRound();
	
	void HandleResultsFinished();
	
	void FinishMatch();
	
	void CommitChaserTime(AJellyPlayerState* Chaser);
	
	void TryStartJellyMatch();
	
	
	bool SelectRandomChaser();
	
	AJellyPlayerState* FindCurrentChaser() const;
	
	uint8 FindAvailableColorIndex() const;
	
	UPROPERTY()
	TObjectPtr<AJellyGameStateBase> JellyGameStateBase;
	
	FTimerHandle WaitingTimerHandle;
	FTimerHandle PhaseTimerHandle;
	FTimerHandle ResultsTimerHandle;
	
	int32 CountdownTimeRemaining = 0;
	int32 RoundTimeRemaining = 0;
	float ChaserPeriodStartTime = 0.f;
	bool bIsTrackingChaserTime = false;
	bool bMatchFlowStarted = false;
	
};
