// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "JellyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class JELLY_API AJellyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	bool IsChaser() const;
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	int32 GetPenaltyPoints() const;
	
	void SetIsChaser(bool bIsNewChaser);
	void AddPenaltyPoint();
	void ResetPenaltyPoints();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly,Category = "Jelly|Round", meta = (AllowPrivateAccess = "true"))
	bool bIsChaser;
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly,Category = "Jelly|Round", meta = (AllowPrivateAccess = "true"))
	int32 PenaltyPoints = 0;
};


