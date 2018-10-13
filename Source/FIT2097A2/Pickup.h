// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

using namespace std;

UCLASS()
class FIT2097A2_API APickup : public AActor
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	APickup();


	//***************************************************************************************************
	//** MYCODE
	//***************************************************************************************************
	/**/

	FString GetPickupName() { return Name; }

	FString GetPickupDisplayText(){ return DisplayTest;}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		FString DisplayTest;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
