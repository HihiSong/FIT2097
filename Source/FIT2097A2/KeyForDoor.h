// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeyForDoor.generated.h"

UCLASS()
class FIT2097A2_API AKeyForDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKeyForDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	FString GetPickupName() { return Name; }

	FString GetPickupDisplayText() { return DisplayTest; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Property)
		FString Name = "Key";

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Property)
		FString DisplayTest = "A pickable key";
};
