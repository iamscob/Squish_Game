// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DashComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JELLY_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDashComponent();
	
	void TryDash();
	
	bool IsDashing() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Dash")
	float DashDistance = 300.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Dash")
	float DashDuration = .3f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Dash")
	float DashCooldown = 5.f;
	
private:
	bool CanDash() const;

	void StartDash(const FVector& RequestedDirection);
	
	void EndDash();
	
	UFUNCTION(Server, Reliable)
	void ServerTryDash(FVector_NetQuantizeNormal RequestedDirection);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartDash(FVector_NetQuantizeNormal DashDirection);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEndDash();
	
	bool bIsDashing = false;
	
	float NextDashTime = 0.f;
	
	float SavedGroundFriction = 0.f;
	
	float SavedBrakingDeceleration = 0.f;
	
	FTimerHandle DashTimerHandle;
	
};
