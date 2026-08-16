// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JellyStatusComponent.generated.h"

class AJellyCharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JELLY_API UJellyStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UJellyStatusComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Status")
	bool bIsChasing = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Status")
	bool bIsStunned = false;
	

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	
	FTimerHandle StunTimerHandle;
	
	FVector PreRagdollLocation;
	FRotator PreRagdollRotation;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintPure, Category = "Status")
	bool IsChasing() const;
	
	UFUNCTION(BlueprintCallable, Category = "Status")
	void SetIsChasing(bool bNewIsChasing);
	
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool ApplyHit(AJellyCharacterBase* Attacker, const FVector& HitDirection, bool bIsThrownHit = false);
	
	UFUNCTION(BlueprintCallable, Category = "Status")
	void RecoverFromStun();
};
