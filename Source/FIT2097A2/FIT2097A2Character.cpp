// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FIT2097A2Character.h"
#include "FIT2097A2Projectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AFIT2097A2Character

AFIT2097A2Character::AFIT2097A2Character(const FObjectInitializer& ObjectIntializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AFIT2097A2Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

void AFIT2097A2Character::Tick(float DeltaTime)
{
	CallMyTrace();
	currentHealth -= 0.01f;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, PickupName);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFIT2097A2Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFIT2097A2Character::OnFire);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFIT2097A2Character::SwitchTraceLine);
	//PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AFIT2097A2Character::OpenDoor_Implementation(CurrentPickup, UGameplayStatics::GetPlayerController(GetWorld(),0)));
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AFIT2097A2Character::serverFunction);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AFIT2097A2Character::clientFunction);
	//PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AFIT2097A2Character::IncreaseKey);
	//PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AFIT2097A2Character::IncreaseFuse);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AFIT2097A2Character::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFIT2097A2Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFIT2097A2Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFIT2097A2Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFIT2097A2Character::LookUpAtRate);

}

void AFIT2097A2Character::OnFire()	
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AFIT2097A2Projectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				// World->SpawnActor<AFIT2097A2Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AFIT2097A2Character::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AFIT2097A2Character::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AFIT2097A2Character::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AFIT2097A2Character::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AFIT2097A2Character::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFIT2097A2Character::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFIT2097A2Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFIT2097A2Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AFIT2097A2Character::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFIT2097A2Character::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AFIT2097A2Character::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AFIT2097A2Character::TouchUpdate);
		return true;
	}
	
	return false;
}

//***************************************************************************************************
//** MYCODE
//***************************************************************************************************

//***************************************************************************************************
//** Trace functions - used to detect items we are looking at in the world
//***************************************************************************************************
//***************************************************************************************************

//***************************************************************************************************
//** Trace() - called by our CallMyTrace() function which sets up our parameters and passes them through
//***************************************************************************************************

bool AFIT2097A2Character::Trace(
	UWorld* World,
	TArray<AActor*>& ActorsToIgnore,
	const FVector& Start,
	const FVector& End,
	FHitResult& HitOut,
	ECollisionChannel CollisionChannel = ECC_Pawn,
	bool ReturnPhysMat = false
) {

	// The World parameter refers to our game world (map/level) 
	// If there is no World, abort
	if (!World)
	{
		return false;
	}

	// Set up our TraceParams object
	FCollisionQueryParams TraceParams(FName(TEXT("My Trace")), true, ActorsToIgnore[0]);

	// Should we simple or complex collision?
	TraceParams.bTraceComplex = true;

	// We don't need Physics materials 
	TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;

	// Add our ActorsToIgnore
	TraceParams.AddIgnoredActors(ActorsToIgnore);

	// When we're debugging it is really useful to see where our trace is in the world
	// We can use World->DebugDrawTraceTag to tell Unreal to draw debug lines for our trace
	// (remove these lines to remove the debug - or better create a debug switch!)
	
	if (TraceLineSwitch == true)
	{
		const FName TraceTag("MyTraceTag");
		World->DebugDrawTraceTag = TraceTag;
		TraceParams.TraceTag = TraceTag;
	}
	

	// Force clear the HitData which contains our results
	HitOut = FHitResult(ForceInit);

	// Perform our trace
	World->LineTraceSingleByChannel
	(
		HitOut,		//result
		Start,	//start
		End, //end
		CollisionChannel, //collision channel
		TraceParams
	);

	// If we hit an actor, return true
	return (HitOut.GetActor() != NULL);
}

//***************************************************************************************************
//** CallMyTrace() - sets up our parameters and then calls our Trace() function
//***************************************************************************************************

void AFIT2097A2Character::CallMyTrace()
{
	// Get the location of the camera (where we are looking from) and the direction we are looking in
	const FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	const FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();

	// How for in front of our character do we want our trace to extend?
	// ForwardVector is a unit vector, so we multiply by the desired distance
	const FVector End = Start + ForwardVector * 256;

	// Force clear the HitData which contains our results
	FHitResult HitData(ForceInit);

	// What Actors do we want our trace to Ignore?
	TArray<AActor*> ActorsToIgnore;

	//Ignore the player character - so you don't hit yourself!
	ActorsToIgnore.Add(this);

	// Call our Trace() function with the paramaters we have set up
	// If it Hits anything
	if (Trace(GetWorld(), ActorsToIgnore, Start, End, HitData, ECC_Visibility, false))
	{
		// Process our HitData
		if (HitData.GetActor())
		{

			//UE_LOG(LogClass, Warning, TEXT("This a testing statement. %s"), *HitData.GetActor()->GetName());
			ProcessTraceHit(HitData);

		}
		else
		{
			// The trace did not return an Actor
			// An error has occurred
			// Record a message in the error log
		}
	}
	else
	{
		// We did not hit an Actor
		ClearPickupInfo();

	}

}

//***************************************************************************************************
//** ProcessTraceHit() - process our Trace Hit result
//***************************************************************************************************
/**/
void AFIT2097A2Character::ProcessTraceHit(FHitResult& HitOut)
{

	// Cast the actor to APickup
	APickup* const TestPickup = Cast<APickup>(HitOut.GetActor());

	if (TestPickup)
	{
		// Keep a pointer to the Pickup
		CurrentPickup = TestPickup;

		// Set a local variable of the PickupName for the HUD
		//UE_LOG(LogClass, Warning, TEXT("PickupName: %s"), *TestPickup->GetPickupName());
		PickupName = TestPickup->GetPickupName();

		// Set a local variable of the PickupDisplayText for the HUD
		//UE_LOG(LogClass, Warning, TEXT("PickupDisplayText: %s"), *TestPickup->GetPickupDisplayText());
		PickupDisplayText = TestPickup->GetPickupDisplayText();
		PickupFound = true;

		//debug
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::Yellow,PickupName);


		
	}
	else
	{
		//UE_LOG(LogClass, Warning, TEXT("TestPickup is NOT a Pickup!"));
		ClearPickupInfo();
	}
}

void AFIT2097A2Character::ClearPickupInfo()
{
	PickupName = "";
	PickupDisplayText = "";
}

void AFIT2097A2Character::SwitchTraceLine()
{
	if(TraceLineSwitch == true)
	{
		TraceLineSwitch = false;
	}
	else
	{
		TraceLineSwitch = true;
	}
}


bool AFIT2097A2Character::serverFunction_Validate() { return true; }

void AFIT2097A2Character::serverFunction_Implementation()
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Call server"));

	if (Role == ROLE_Authority)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("have auth"));
		//if(CurrentPickup==nullptr)
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("None pointer"));

		if (IsValid(CurrentPickup))
		{
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("ok"));

			//OpenDoor();
		}
		//else {
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("invalid"));

		//}
	}
}

bool AFIT2097A2Character::myDestroy_Validate(AActor* myActor) { return true; }

void AFIT2097A2Character::myDestroy_Implementation(AActor* myActor)
{
	if (Role == ROLE_Authority) {
		if (IsValid(myActor)) {
			myActor->Destroy(true);
		}
	}
}

bool AFIT2097A2Character::OpenDoor_Validate(AActor* myActor) { return true; }

void AFIT2097A2Character::OpenDoor_Implementation(AActor* myActor)
{
	if (Role == ROLE_Authority) 
	{
		if (IsValid(myActor)) {
			FOutputDeviceNull ar;
			myActor->CallFunctionByNameWithArguments(TEXT("OpenDoor"), ar, NULL, true);
		}
	}

	//if (Role == ROLE_Authority) 
	//{
		//if (IsValid(CurrentPickup))
		//{
			/*GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, PickupName);
			if (PickupName == "MR DOOR")
			{
				FOutputDeviceNull ar;
				CurrentPickup->CallFunctionByNameWithArguments(TEXT("OpenDoor"), ar, NULL, true);
			}*/
			//if (PickupName == "Key")
			//{
			//	numOfKey += 1;
			//	myDestroy(CurrentPickup);
			//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Test"));

			//	//FOutputDeviceNull ar;
			//	//CurrentPickup->CallFunctionByNameWithArguments(TEXT("DestroyActor"), ar, NULL, true);
			//}
			//if (PickupName == "Fuse")
			//{
			//	numOfFuse += 1;
			//	myDestroy(CurrentPickup);
			//	//FOutputDeviceNull ar;
			//	//CurrentPickup->CallFunctionByNameWithArguments(TEXT("DestroyActor"), ar, NULL, true);
			//}
			//if (PickupName == "HealthPack")
			//{
			//	currentHealth += 20;
			//}
			/*if (PickupName == "MR KEYDOOR")
			{
				if (numOfKey >= 1)
				{
					FOutputDeviceNull ar;
					CurrentPickup->CallFunctionByNameWithArguments(TEXT("OpenDoor"), ar, NULL, true);
					numOfKey -= 1;
				}
			}
			if (PickupName == "MR FUSEDOOR")
			{
				if (numOfFuse >= 1)
				{
					FOutputDeviceNull ar;
					CurrentPickup->CallFunctionByNameWithArguments(TEXT("OpenDoor"), ar, NULL, true);
					numOfFuse -= 1;
				}
			}
			if (PickupName == "MR REMOTEDOOR")
			{
					FOutputDeviceNull ar;
					CurrentPickup->CallFunctionByNameWithArguments(TEXT("OpenDoor"), ar, NULL, true);
					numOfFuse -= 1;
			}*/

		//}
	//}
	
	//const FString command = FString::Printf(TEXT("OpenDoor %s"),*Actor->);
	
}




void AFIT2097A2Character::clientFunction()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, PickupName);
	if (PickupName == "Key")
	{
		numOfKey += 1;
		myDestroy(CurrentPickup);
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Test"));

		//FOutputDeviceNull ar;
		//CurrentPickup->CallFunctionByNameWithArguments(TEXT("DestroyActor"), ar, NULL, true);
	}
	if (PickupName == "Fuse")
	{
		numOfFuse += 1;
		myDestroy(CurrentPickup);
		//FOutputDeviceNull ar;
		//CurrentPickup->CallFunctionByNameWithArguments(TEXT("DestroyActor"), ar, NULL, true);
	}
	if (PickupName == "HealthPack")
	{
		currentHealth += 20;
		myDestroy(CurrentPickup);
	}
	if (PickupName == "Remote")
	{
		OpenDoor(CurrentPickup);
		//FOutputDeviceNull ar;
		//CurrentPickup->CallFunctionByNameWithArguments(TEXT("DestroyActor"), ar, NULL, true);
	}
	if (PickupName == "MR KEYDOOR")
	{
		if (numOfKey >= 1)
		{
			OpenDoor(CurrentPickup);
			numOfKey -= 1;
		}
	}
	if (PickupName == "MR FUSEDOOR")
	{
		if (numOfFuse >= 1)
		{
			OpenDoor(CurrentPickup);
			numOfFuse -= 1;
		}
	}
	if (PickupName == "MR REMOTEDOOR")
	{
		OpenDoor(CurrentPickup);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, PickupName);
	if (PickupName == "MR DOOR")
	{
		OpenDoor(CurrentPickup);
	}
}