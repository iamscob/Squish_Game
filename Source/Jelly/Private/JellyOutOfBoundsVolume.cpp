// Fill out your copyright notice in the Description page of Project Settings.


#include "JellyOutOfBoundsVolume.h"

#include "JellyGameModeBase.h"
#include "Components/BoxComponent.h"
#include "Character/JellyCharacterBase.h"
#include "EquippableToolBase.h"
#include "Engine/TargetPoint.h"


// Sets default values
AJellyOutOfBoundsVolume::AJellyOutOfBoundsVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(1000.f,1000.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AJellyOutOfBoundsVolume::BeginPlay()
{
	Super::BeginPlay();
	
	if (!TriggerBox) return;

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AJellyOutOfBoundsVolume::OnTriggerBeginOverlap);
	}
	else
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AJellyOutOfBoundsVolume::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor) return;

	if (AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(OtherActor))
	{
		AJellyGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AJellyGameModeBase>() : nullptr;

		if (GameMode)
		{
			GameMode->RespawnCharacter(Character);
		} return;
	}
	if (AEquippableToolBase* Tool = Cast<AEquippableToolBase>(OtherActor))
	{
		if (!ToolRespawnPoint) return;
		
			Tool->ReturnToArena(ToolRespawnPoint->GetActorTransform());
	}
}

