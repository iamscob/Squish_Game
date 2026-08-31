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
	
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Player")
	uint8 GetPlayerColorIndex() const;
	
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	bool IsChaser() const;
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Round")
	float GetChaserTime() const;
	
	void SetIsChaser(bool bIsNewChaser);
	void AddChaserTime(float Seconds);
	void ResetChaserTime();
	void SetPlayerColorIndex(uint8 NewColorIndex);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly,Category = "Jelly|Round", meta = (AllowPrivateAccess = "true"))
	bool bIsChaser = false;
	
	UPROPERTY(Replicated,VisibleAnywhere,BlueprintReadOnly,Category = "Jelly|Round", meta = (AllowPrivateAccess = "true"))
	float ChaserTimeSeconds = 0;
	
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerColorIndex,VisibleAnywhere, BlueprintReadOnly, Category = "Jelly|Player", meta=(AllowPrivateAccess = "true"))
	uint8 PlayerColorIndex = 255;
	
	UFUNCTION()
	void OnRep_PlayerColorIndex();
};


