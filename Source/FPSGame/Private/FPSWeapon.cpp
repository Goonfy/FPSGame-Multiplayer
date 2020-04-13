// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "FPSCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AFPSWeapon::AFPSWeapon()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;
	
	// Create a gun mesh component
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	GunMeshComponent->SetupAttachment(RootComponent);
	GunMeshComponent->CastShadow = true;

	NoiseEmitterComponent = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));

	MyOwner = Cast<AFPSCharacter>(GetOwner());
}

void AFPSWeapon::Fire()
{
	if (MyOwner)
	{
		float LineTraceDistance = 1500.0f;

		FVector CameraLocation = MyOwner->GetFirstPersonCameraComponent()->GetComponentLocation();
		FRotator CameraRotation = MyOwner->GetFirstPersonCameraComponent()->GetComponentRotation();
		
		FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * LineTraceDistance);
		
		// additional trace parameters
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(MyOwner);

		//Re-initialize hit info
		FHitResult Hit;
		bool bIsHit = GetWorld()->LineTraceSingleByChannel(
			Hit,      // FHitResult object that will be populated with hit info
			CameraLocation,      // starting position
			TraceEnd,        // end position
			ECC_Visibility,  // collision channel
			QueryParams      // additional trace settings
		);

		if (bIsHit && Hit.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("We hit something"));
			// start to end, green, will lines always stay on, depth priority, thickness of line
			DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Green, false, 5.f, ECC_WorldStatic, 1.f);

			UE_LOG(LogTemp, Warning, TEXT("Hit actor name %s"), *Hit.GetActor()->GetName());
			UE_LOG(LogTemp, Warning, TEXT("Hit actor distance %s"), *FString::SanitizeFloat(Hit.Distance));
			DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(10.f, 10.f, 10.f), FColor::Red, false, 5.f, ECC_WorldStatic, 5.f);

			UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Hit.GetActor()->GetRootComponent());
			if (PrimComp && PrimComp->IsSimulatingPhysics())
			{
				PrimComp->AddImpulseAtLocation(CameraRotation.Vector() * 1000.f * PrimComp->GetMass(), Hit.ImpactPoint);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Nothing was hit"));
			// start to end, purple, will lines always stay on, depth priority, thickness of line
			DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Purple, false, 5.f, ECC_WorldStatic, 1.f);
		}

		MakeNoise(1.0f, MyOwner);

		// try and play the sound if specified
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = MyOwner->GetMesh1P()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "Arms", 0.0f);
			}
		}
	}
}