// Fill out your copyright notice in the Description page of Project Settings.

#include "JelloCombatComponent.h"
#include "Character/JellyCharacterBase.h"
#include "JellyStatusComponent.h"
#include "DrawDebugHelpers.h"



// Sets default values for this component's properties
UJelloCombatComponent::UJelloCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);

	// ...
}


// Called when the game starts
void UJelloCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UJelloCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

//Melee Attack Fun

void UJelloCombatComponent::MeleeAttack()
{
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());

	if (!OwnerCharacter) return;
	if (OwnerCharacter->HasAuthority())
	{
		PerformMeleeAttack();
		return;
	}
	ServerMeleeAttack();
}

void UJelloCombatComponent::PerformMeleeAttack()
{
	AJellyCharacterBase* OwnerCharacter = Cast<AJellyCharacterBase>(GetOwner());
	if (!OwnerCharacter) return;
	UJellyStatusComponent* OwnerStatus = OwnerCharacter->FindComponentByClass<UJellyStatusComponent>();
	
	if (OwnerStatus && OwnerStatus->bIsStunned)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,TEXT("Stunned"));
		return;
	}
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastMeleeTime < MeleeCooldownTime)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,TEXT("Cooldown"));
		return;
		
	}
	LastMeleeTime = CurrentTime;
	
	FVector Start = OwnerCharacter->GetActorLocation() + FVector(0.f,0.f,80.f);
	FVector End = Start + (OwnerCharacter->GetActorForwardVector() * 300.f);
	float Radius = 70.f;
	
	TArray<FHitResult> HitResults;
	FCollisionQueryParams  QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
		);
	
	FColor DebugColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 0.5f, 0, 3.f);
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AJellyCharacterBase* HitCharacter = Cast<AJellyCharacterBase>(Hit.GetActor());
			if (HitCharacter && HitCharacter != OwnerCharacter)
			{
				UJellyStatusComponent* HitStatus = HitCharacter->FindComponentByClass<UJellyStatusComponent>();
				if (HitStatus)
				{
					const bool bHitApplied = HitStatus->ApplyHit(OwnerCharacter, OwnerCharacter->GetActorForwardVector(), false);
					if (bHitApplied)
					{
						GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,TEXT("Melee Hit"));
						break;
					}
				}
			}
			
		}
	}
}

void UJelloCombatComponent::ServerMeleeAttack_Implementation()
{
	PerformMeleeAttack();
}





