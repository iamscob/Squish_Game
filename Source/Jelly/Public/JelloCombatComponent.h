// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JelloCombatComponent.generated.h"

class AJellyCharacterBase; 

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JELLY_API UJelloCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UJelloCombatComponent();

	// Combat Variables
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Combat")
	float MeleeCooldownTime = .5f;
	

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	
	float LastMeleeTime =0.f;
	
	UFUNCTION(Server,Reliable)
	void ServerMeleeAttack();
	
	void PerformMeleeAttack();


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void MeleeAttack();
	
		
};
