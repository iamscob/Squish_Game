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
	
	PickupMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	
	PickupCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	check (PickupCollisionComponent != nullptr);
	
	PickupCollisionComponent->SetupAttachment(PickupMeshComponent);
	PickupCollisionComponent->SetBoxExtent(FVector(32.f,32.f,32.f));
	PickupCollisionComponent->SetGenerateOverlapEvents(true);
	PickupCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();

	if (!PickupCollisionComponent)
	{
	return;;	
	}
	
	PickupCollisionComponent->OnComponentBeginOverlap.RemoveAll(this);
	PickupCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnBoxBeginOverlap);
	PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InitializePickup();
	
	AnimStartOffsetZ = GetActorLocation().Z;
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ReferenceItem && ReferenceItem->ItemType == EItemType::Consumable)
	{
		FRotator CurrentRotation = PickupMeshComponent->GetRelativeRotation();
		CurrentRotation.Yaw += RotationSpeed * DeltaTime;
		PickupMeshComponent->SetRelativeRotation(CurrentRotation);
		float Time = GetWorld()->GetTimeSeconds();
		float FloatingOffset = FMath::Sin(Time * FloatingSpeed) * FloatingAmplitude;
		
		FVector NewLocation = GetActorLocation();
		NewLocation.Z = AnimStartOffsetZ + FloatingOffset;
		SetActorLocation(NewLocation);
	}
}

// Item Initialization Fun
void APickupBase::InitializePickup()
{
	if (!PickupMeshComponent || !PickupCollisionComponent)
	{	
		return;
	}
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
	if (ReferenceItem && ReferenceItem->ItemType == EItemType::Tool)
	{
		PickupMeshComponent->SetSimulatePhysics(true);
		PickupMeshComponent->SetCollisionEnabled((ECollisionEnabled::QueryAndPhysics));
		PickupMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
		PickupMeshComponent->SetMassOverrideInKg(NAME_None,50.0f);
	}
	else
	{
		PickupMeshComponent->SetSimulatePhysics(false);
		PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		PickupMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
	}


	
	if (PickupMeshComponent->GetStaticMesh())
	{
		FBoxSphereBounds Bounds = PickupMeshComponent->GetStaticMesh()->GetBounds();
		FVector BoxExtent = Bounds.BoxExtent*BoxScale;
		PickupCollisionComponent->SetBoxExtent(BoxExtent);
	}
	if (PickupOutlineMaterial)
	{
		PickupMeshComponent->SetOverlayMaterial(PickupOutlineMaterial);
	}
}

// Overlap implementation
	
void APickupBase::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Attempting a pickup collision"));
 
	// Checking if Character's overlapping
	AJellyCharacterBase* Character = Cast<AJellyCharacterBase>(OtherActor);
	if (Character != nullptr)
	{
		bool bPickedUp = Character->GiveItem(ReferenceItem);
		if (bPickedUp)
		{
		K2_DestroyActor();
		}
		
	}
	
}

