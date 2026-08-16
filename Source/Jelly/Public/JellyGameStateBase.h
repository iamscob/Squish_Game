// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "JellyGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EJellyRoundPhase : uint8
{
	Waiting UMETA(DisplayName = "Waiting"),
	Countdown UMETA(DisplayName = "Countdown"),
	Playing UMETA(DisplayName = "Playing"),
	Results UMETA(DisplayName = "Results"),
	MatchFinished UMETA(DisplayName = "MatchFinished")	
};

UCLASS()
class JELLY_API AJellyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	EJellyRoundPhase GetRoundPhase() const;
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	int32 GetCurrentRound() const;
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	int32 GetTotalRounds() const;
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	int32 GetRemainingTime() const;
	
	void SetRoundPhase(EJellyRoundPhase NewPhase);
	void SetCurrentRound(int32 NewCurrentRound);
	void SetTotalRounds(int32 NewTotalRounds);
	void SetRemainingTime (int32 NewRemaningTime);
	
protected:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly, Category="Jelly|Round", meta = (AllowPrivateAccess = "true"))
	EJellyRoundPhase RoundPhase = EJellyRoundPhase::Waiting;
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly, Category="Jelly|Round", meta = (AllowPrivateAccess = "true"))
	int32 CurrentRound = 0;
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly, Category="Jelly|Round", meta = (AllowPrivateAccess = "true"))
	int32 TotalRounds = 3;
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly, Category="Jelly|Round", meta = (AllowPrivateAccess = "true"))
	int32 RemaningTime = 0;
};
