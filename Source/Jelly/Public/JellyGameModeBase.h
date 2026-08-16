// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JellyGameModeBase.generated.h"

class AJellyGameStateBase;
class AJellyCharacterBase;

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
	
private:
	
	void StartCountdown();
	void HandleCountdownTick();
	
	void StartRound();
	void HandleRoundTick();
	
	void EndRound();
	
	bool SelectRandomChaser();
	
	UPROPERTY()
	TObjectPtr<AJellyGameStateBase> JellyGameStateBase;
	
	FTimerHandle WaitingTimerHandle;
	FTimerHandle PhaseTimerHandle;
	
	int32 CountdownTimeRemaining = 0;
	int32 RoundTimeRemaining = 0;
	
};
