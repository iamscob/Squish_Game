// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableToolBase.h"
#include "Character/JellyCharacterBase.h"
#include "InputMappingContext.h"

// Sets default values
AEquippableToolBase::AEquippableToolBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ToolMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ToolMesh"));
	check(ToolMeshComponent!=nullptr);

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

void AEquippableToolBase::Use()
{
	
}

void AEquippableToolBase::BindInputAction(const UInputAction* ActionToBind)
{
	
}
