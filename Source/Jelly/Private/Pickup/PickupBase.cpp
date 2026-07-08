// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/PickupBase.h"
#include "ItemDefinition.h"
#include "Data/ItemData.h"

// Sets default values
APickupBase::APickupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PickupMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	check (PickupMeshComponent != nullptr);
	SetRootComponent(PickupMeshComponent);
	
	PickupCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	check (PickupCollisionComponent != nullptr);
	
	PickupCollisionComponent->SetupAttachment(PickupMeshComponent);
	PickupCollisionComponent->SetSphereRadius(32.f);
	PickupCollisionComponent->SetGenerateOverlapEvents(true);
	PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	
	PickupCollisionComponent->OnComponentBeginOverlap.RemoveAll(this);
	PickupCollisionComponent->OnComponentBeginOverlap.AddDynamic(
		this, &APickupBase::OnSphereBeginOverlap
		);
	PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InitializePickup();
	
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Item Initialization Fun
void APickupBase::InitializePickup()
{
	const FSoftObjectPath TablePath = PickupDataTable.ToSoftObjectPath();
	if (!TablePath.IsNull() && !PickupItemID.IsNone())
	{
		UDataTable* LoadedDataTable = PickupDataTable.IsValid()
		? PickupDataTable.Get()
		: PickupDataTable.LoadSynchronous();
		
		if (!LoadedDataTable)
		{
			return;
		}
		
		
		const FItemData* ItemDataRow = LoadedDataTable->FindRow<FItemData>(PickupItemID, PickupItemID.ToString());
		if (!ItemDataRow)
		{
			return;
		}
		UItemDefinition* TempItemDefinition = ItemDataRow->ItemBase.IsValid()
		? ItemDataRow->ItemBase.Get()
		: ItemDataRow->ItemBase.LoadSynchronous();
		
		if (!TempItemDefinition)
		{
			return;
		}
		
		ReferenceItem = TempItemDefinition->CreateItemCopy(this);
		
		UStaticMesh* LoadedMesh = TempItemDefinition->WorldMesh.IsValid()
		? TempItemDefinition->WorldMesh.Get()
		: TempItemDefinition->WorldMesh.LoadSynchronous();
		if (LoadedMesh)
		{
			PickupMeshComponent->SetStaticMesh(LoadedMesh);
		}
		
	}
	
	PickupMeshComponent->SetVisibility(true);
	PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
}

// Overlap implementation
	
void APickupBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Attempting a pickup collision"));
 
	// Checking if Character's overlapping
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(OtherActor);
	if (Character != nullptr)
	{
		PickupMeshComponent->SetVisibility(false);
		PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	if (bShouldRespawn)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandler, this, &APickupBase::InitializePickup, RespawnTime, false);
	}
	
}
void APickupBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName ChangedPropertyName = PropertyChangedEvent.Property 
	? PropertyChangedEvent.Property->GetFName() 
	: NAME_None;
	
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(APickupBase, PickupItemID)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(APickupBase, PickupDataTable))
	{
		InitializePickup();
	}
	
};