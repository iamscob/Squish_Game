// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableToolBase.h"
#include "Character/JellyCharacterBase.h"
#include "InputMappingContext.h"
#include "JellyStatusComponent.h"

// Sets default values
AEquippableToolBase::AEquippableToolBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	ToolMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
	check(ToolMeshComponent!=nullptr);
	SetRootComponent(ToolMeshComponent);
	
	ToolMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ToolMeshComponent->SetGenerateOverlapEvents(false);
	
	ToolMeshComponent->SetNotifyRigidBodyCollision(true);
	ToolMeshComponent->OnComponentHit.AddDynamic(this, &AEquippableToolBase::OnToolHit);
}

// Called when the game starts or when spawned
void AEquippableToolBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEquippableToolBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEquippableToolBase::OnToolHit(
	UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& HitResult
	)
{
	if (bHasProcessedHit || !Thrower || !OtherActor) return;
	
	AJellyCharacterBase* HitCharacter = Cast<AJellyCharacterBase>(OtherActor);
	
	if (!HitCharacter || HitCharacter == Thrower) return;
	
	UJellyStatusComponent* HitStatus = HitCharacter->FindComponentByClass<UJellyStatusComponent>();
	
	if (!HitStatus) return;
	
	FVector HitDirection = -NormalImpulse.GetSafeNormal();
	if (HitDirection.IsNearlyZero())
	{
		HitDirection = -HitResult.ImpactNormal.GetSafeNormal();
	}
	const bool bHitApplied = HitStatus->ApplyHit(Thrower, HitDirection, true);
	if (!bHitApplied) return;
	bHasProcessedHit = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,TEXT("Throw Hit"));
	Destroy();
}
	


