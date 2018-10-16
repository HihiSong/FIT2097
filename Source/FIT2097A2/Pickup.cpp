// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Net/UnrealNetwork.h"
#include "FIT2097A2.h"



// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	bReplicateMovement = true;

	//change everytime
	//UStaticMeshComponent->SetMobility(EComponentMobility::Movable);
	//GetStaticMesh2()->SetMobility(EComponentMobility::Movable);
}

// Called when the game starts or when spawned
/**/
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::OnRep_IsActive()
{
}

void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APickup, bIsActive);
}

bool APickup::IsActive()
{
	return bIsActive;
}

void APickup::setActive(bool NewState)
{
	if (Role == ROLE_Authority)
	{
		bIsActive = NewState;
	}
}

