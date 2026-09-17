

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
class UDashComponent;
class UDecalComponent;
class UAnimMontage;

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
	TObjectPtr<UInputAction> MeleeAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Movement")
	float RunnerWalkSpeed = 600.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Movement")
	float ChaserWalkSpeed = 615.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Throw")
	TObjectPtr<UAnimMontage> ThrowMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jelly|Combat")
	TObjectPtr<UAnimMontage> MeleeMontage;

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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDashComponent> DashComponent;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Jelly|Color")
	int32 PlayerColorMaterialIndex = 0;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Jelly|Color")
	FName PlayerColorParameterName = TEXT("PlayerColor");
	
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic>PlayerColorMaterial;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jelly|Indicators")
	TObjectPtr<UDecalComponent> PlayerRingDecal;
	
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic>PlayerRingMaterial;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jelly|Indicators")
	TObjectPtr<UDecalComponent> ThrowAimDecal;
	
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ThrowAimMaterial;	
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;
	
	virtual void PawnClientRestart() override;
	


private:

	void RemoveToolMappingContext(AEquippableToolBase* Tool);
	
	UFUNCTION()
	void OnRep_EquippedTool();
	
	UFUNCTION(Server, Reliable)
	void ServerThrow(AEquippableToolBase* ExpectedTool, FVector_NetQuantizeNormal ThrowDirection);
	
	UFUNCTION(Server, Reliable)
	void ServerPlayMeleeMontage();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayMeleeMontage();
	
	void PerformThrow(const FVector& ThrowDirection);
	
	void AddToolMappingContext(AEquippableToolBase* Tool);
	
	void HandleJumpStarted();
	
	void HandleJumpEnded();
	
	void Dash();
	
	bool bIsPreparingThrow = false;	
	
	TWeakObjectPtr<AEquippableToolBase> LocallyMappedTool;
	
	void SetThrowAimVisible(bool bVisible);
	
	float ThrowAnimAlpha = 0.f;
	
	bool bMeleeHitPending = false;
	
	bool bThrowReleasePending = false;
	
	FVector PendingThrowDirection = FVector::ForwardVector;
	
	TWeakObjectPtr<AEquippableToolBase> PendingThrowTool;
	
	UFUNCTION(Server, Reliable)
	void ServerStartThrowMontage(AEquippableToolBase* ExpectedTool);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartThrowMontage();
	
	UFUNCTION(Server, Reliable)
	void ServerResumeThrowMontage(AEquippableToolBase* ExpectedTool);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastResumeThrowMontage();
	
	UFUNCTION(Server, Reliable)
	void ServerCancelThrowMontage();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastCancelThrowMontage();
	
public:
	
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, Category = "Jelly|Throw" )
	void HandleThrowReady();
	
	UFUNCTION(BlueprintCallable, Category = "Jelly|Throw" )
	void HandleThrowRelease();
	
	UFUNCTION()
	void StartPreparingThrow();
	
	UFUNCTION()
	void ReleasePreparedThrow();
	
	UFUNCTION()
	void CancelPreparingThrow();
	
	UFUNCTION()
	void MeleeAttack();

	UFUNCTION()
	bool AttachTool(UEquippableToolDefinition* ToolDefinition);
	
	UFUNCTION()
	bool AttachExistingTool(AEquippableToolBase* ToolToEquip);
	
	UFUNCTION()
	bool GiveItem(UItemDefinition* ItemDefinition);
	
	void ApplyPlayerColor();
	
	void ApplyRoleMovementSpeed();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Jelly|Indicators")
	void SetChaserIndicatorVisible(bool bVisible);
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Tool")
	bool HasEquippedTool() const;
	
	UFUNCTION(BlueprintPure, Category="Jelly|Match")
	bool CanUseInput() const;
	
	UFUNCTION(BlueprintPure, Category = "Jelly|Tool")
	AEquippableToolBase* GetEquippedTool() const;
	
	UFUNCTION(BlueprintCallable, Category = "Jelly|Combat")
	void HandleMeleeHit();
	
	void InterruptActions();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastInterruptActions();
};
