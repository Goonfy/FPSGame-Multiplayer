// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSCharacter.h"
#include "FPSGrenade.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AFPSCharacter::AFPSCharacter()
{
	// Create a CameraComponent	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(0, 0, BaseEyeHeight)); // Position the camera
	CameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1PComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	Mesh1PComponent->SetupAttachment(CameraComponent);
	Mesh1PComponent->CastShadow = false;
	Mesh1PComponent->SetRelativeRotation(FRotator(2.0f, -15.0f, 5.0f));
	Mesh1PComponent->SetRelativeLocation(FVector(0, 0, -160.0f));

	// Create a gun mesh component
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	GunMeshComponent->CastShadow = false;
	GunMeshComponent->SetupAttachment(Mesh1PComponent, "GripPoint");
}


void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::Fire);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &AFPSCharacter::ThrowGrenade);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}


void AFPSCharacter::Fire()
{
	float LineTraceDistance = 1500.f;

	FVector StartTrace = GetFirstPersonCameraComponent()->GetComponentLocation();
	FVector ForwardVector = GetFirstPersonCameraComponent()->GetForwardVector();
	FVector EndTrace = StartTrace + (ForwardVector * LineTraceDistance);
	
	// additional trace parameters
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.AddIgnoredActor(this);

	//Re-initialize hit info
	FHitResult HitDetails = FHitResult(ForceInit);

	bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		HitDetails,      // FHitResult object that will be populated with hit info
		StartTrace,      // starting position
		EndTrace,        // end position
		ECC_GameTraceChannel3,  // collision channel - 3rd custom one
		TraceParams      // additional trace settings
	);

	if (bIsHit && HitDetails.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("We hit something"));
		// start to end, green, will lines always stay on, depth priority, thickness of line
		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 5.f, ECC_WorldStatic, 1.f);

		UE_LOG(LogTemp, Warning, TEXT("Hit actor name %s"), *HitDetails.GetActor()->GetName());
		UE_LOG(LogTemp, Warning, TEXT("Hit actor distance %s"), *FString::SanitizeFloat(HitDetails.Distance));
		DrawDebugBox(GetWorld(), HitDetails.ImpactPoint, FVector(10.f, 10.f, 10.f), FColor::Red, false, 5.f, ECC_WorldStatic, 5.f);

		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HitDetails.GetActor()->GetRootComponent());
		if (PrimComp && PrimComp->IsSimulatingPhysics())
		{
			PrimComp->AddImpulseAtLocation(ForwardVector * 1000.f * PrimComp->GetMass(), HitDetails.ImpactPoint);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing was hit"));
  		// start to end, purple, will lines always stay on, depth priority, thickness of line
  		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Purple, false, 5.f, ECC_WorldStatic, 1.f);
	}

	// try and play the sound if specified
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1PComponent->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "Arms", 0.0f);
		}
	}
}

void AFPSCharacter::ThrowGrenade()
{
	if (GrenadeClass)
	{
		FVector MuzzleLocation = GunMeshComponent->GetSocketLocation("Muzzle");
		FRotator MuzzleRotation = GunMeshComponent->GetSocketRotation("Muzzle");

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<AFPSGrenade>(GrenadeClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
	
		if (ThrowSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ThrowSound, GetActorLocation());
		}

		if (ThrowAnimation)
		{
			UAnimInstance* AnimInstance = Mesh1PComponent->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->PlaySlotAnimationAsDynamicMontage(ThrowAnimation, "Arms", 0.0f);
			}
		}
	}
}

void AFPSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}