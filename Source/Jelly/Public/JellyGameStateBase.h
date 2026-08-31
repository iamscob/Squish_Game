// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "JellyGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EJellyMatchPhase : uint8
{
	Waiting UMETA(DisplayName = "Waiting"),
	Countdown UMETA(DisplayName = "Countdown"),
	Playing UMETA(DisplayName = "Playing"),
	Results UMETA(DisplayName = "Results"),
};

UCLASS()
class JELLY_API AJellyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	EJellyMatchPhase GetMatchPhase() const;
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	int32 GetRemainingTime() const;
	
	void SetMatchPhase(EJellyMatchPhase NewPhase);
	void SetRemainingTime (int32 NewRemaningTime);
	
protected:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly, Category="Jelly|Round", meta = (AllowPrivateAccess = "true"))
	EJellyMatchPhase MatchPhase = EJellyMatchPhase::Waiting;
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly, Category="Jelly|Round", meta = (AllowPrivateAccess = "true"))
	int32 RemaningTime = 0;
};
