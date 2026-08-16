// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimBlueprint.h"
#include "Components/StaticMeshComponent.h"
#include "EquippableToolBase.generated.h"

class AJellyCharacterBase;
class UInputAction;
class UInputMappingContext;

UCLASS(BlueprintType, Blueprintable)
class JELLY_API AEquippableToolBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEquippableToolBase();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TObjectPtr<UAnimBlueprint> PickupToolAnim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ToolMeshComponent;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TObjectPtr<AJellyCharacterBase> OwningCharacter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInputMappingContext> ToolMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Scale")
	float WorldScale = .7f;
	
	UPROPERTY()
	bool bThrowerWasChasing = false;
	
	
	
	UPROPERTY()
	TObjectPtr<AJellyCharacterBase> Thrower;
		
	UFUNCTION()
	void OnToolHit(
		UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& HitResult
		);
	
private:
	UPROPERTY()
		bool bHasProcessedHit = false;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
