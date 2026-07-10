// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/SpringArmComponent.h"
#include "JellyCharacterBase.generated.h"

class UAnimBlueprint;
class UInputMappingContext;
class UInputAction;
class UInputComponent;
class UItemDefinition;
class UEquippableToolDefinition;
class AEquippableToolBase;
class UInventoryComponent;

UCLASS()
class JELLY_API AJellyCharacterBase : public ACharacter
{
	GENERATED_BODY()

public: 
	// Sets default values for this character's properties
	AJellyCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Input Actions Declaration
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> ThirdPersonContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Input")
	TObjectPtr<UInputAction> UseAction;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tools")
	TObjectPtr<AEquippableToolBase> EquippedTool;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(EditAnywhere, Category= "Camera")
	FVector CameraOffset = FVector(2.8f, 300.0f, 75.0f);
	
	UPROPERTY(EditAnywhere, Category= "Camera")
	float FieldOfView = 90.0f;
	
	UPROPERTY(EditAnywhere, Category= "Camera")
	float Scale = 0.6f;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	
	// Inventory & Equipment Declaration
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> InventoryComponent;
	
	UFUNCTION()
	bool IsToolAlreadyOwned(UEquippableToolDefinition* ToolDefinition);
	
	UFUNCTION()
	bool AttachTool(UEquippableToolDefinition* ToolDefinition);
	
	UFUNCTION()
	bool GiveItem(UItemDefinition* ItemDefinition);
};
