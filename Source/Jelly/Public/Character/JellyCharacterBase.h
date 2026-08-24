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
class UJellyStatusComponent;
class UJelloCombatComponent;
class UMaterialInstanceDynamic;

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
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Input")
	TObjectPtr<UInputAction> ThrowAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input")
	TObjectPtr<UInputAction> DropAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input")
	TObjectPtr<UInputAction> MeleeAction;
	
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedTool,VisibleAnywhere, BlueprintReadOnly, Category = "Tools")
	TObjectPtr<AEquippableToolBase> EquippedTool;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
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
	
	
	
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> InventoryComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components")
	TObjectPtr<UJellyStatusComponent> StatusComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components")
	TObjectPtr<UJelloCombatComponent> CombatComponent;
	
	
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Jelly|Color")
	int32 PlayerColorMaterialIndex = 0;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Jelly|Color")
	FName PlayerColorParameterName = TEXT("PlayerColor");
	
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic>PlayerColorMaterial;
	
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;

private:

	void RemoveToolMappingContext(AEquippableToolBase* Tool);
	
	UFUNCTION()
	void OnRep_EquippedTool();
	
	UFUNCTION(Server, Reliable)
	void ServerThrow();
	
	void PerformThrow();
	
	void AddToolMappingContext(AEquippableToolBase* Tool);
	
	TWeakObjectPtr<AEquippableToolBase> LocallyMappedTool;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	UFUNCTION()
	void Throw();
	
	UFUNCTION()
	void Drop();
	
	UFUNCTION()
	void MeleeAttack();
	
	UFUNCTION()
	bool AttachTool(UEquippableToolDefinition* ToolDefinition);
	
	UFUNCTION()
	bool GiveItem(UItemDefinition* ItemDefinition);
	
	void ApplyPlayerColor();
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Tool")
	bool HasEquippedTool() const;
	
	
	


	

};
