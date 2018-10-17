// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Property)
		FString Name = "MR DOOR";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Property)
		FString DisplayTest = "A pickable object";

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	bool bIsActive;

	UFUNCTION()
	virtual void OnRep_IsActive();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure)
		bool IsActive();

	UFUNCTION(BlueprintCallable)
		void setActive(bool NewState);
};
