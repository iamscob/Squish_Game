
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Character/JellyCharacterBase.h"
#include "PickupBase.generated.h"

class UItemDefinition;

UCLASS(BlueprintType, Blueprintable)
class JELLY_API APickupBase : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	APickupBase();
	
	void InitializePickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
		
	// Pickup Settings
	
	UPROPERTY(EditInstanceOnly, Category = "Pickup | Item Table")
	FName PickupItemID;
	UPROPERTY(EditInstanceOnly, Category = "Pickup | Item Table")
	TSoftObjectPtr<UDataTable> PickupDataTable; 
	
	UPROPERTY(VisibleAnywhere, Category = "Pickup | Reference Item")
	TObjectPtr<UItemDefinition> ReferenceItem;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Pickup | Mesh")
	TObjectPtr<UStaticMeshComponent> PickupMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Collision Box")
	TObjectPtr<UBoxComponent> PickupCollisionComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Visuals")
	TObjectPtr<UMaterialInterface> PickupOutlineMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Box Scale Multiplier")
	float BoxScale = 1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Animation")
	float RotationSpeed = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Animation")
	float FloatingAmplitude = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Animation")
	float FloatingSpeed = 2.f;
	
	float AnimStartOffsetZ;
	
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};


