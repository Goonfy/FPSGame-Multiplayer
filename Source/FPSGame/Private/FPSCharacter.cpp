// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "FPSAIGuard.h"
#include "FPSProjectile.h"
#include "Net/UnrealNetwork.h"

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

	NoiseEmitterComponent = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::Fire);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &AFPSCharacter::Throw);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled())
	{
		FRotator NewRot = GetFirstPersonCameraComponent()->GetRelativeRotation();
		NewRot.Pitch = RemoteViewPitch * 360.0f / 255.0f;

		GetFirstPersonCameraComponent()->SetRelativeRotation(NewRot);
	}
}

void AFPSCharacter::Fire()
{
	//ServerFire();

	MakeNoise(1.0f, this);

	float LineTraceDistance = 1500.0f;

	FVector CameraLocation = GetFirstPersonCameraComponent()->GetComponentLocation();
	FRotator CameraRotation = GetFirstPersonCameraComponent()->GetComponentRotation();
	
	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * LineTraceDistance);
	
	// additional trace parameters
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(this);

	//Re-initialize hit info
	FHitResult Hit;
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		Hit,      // FHitResult object that will be populated with hit info
		CameraLocation,      // starting position
		TraceEnd,        // end position
		ECC_Visibility,  // collision channel
		QueryParams      // additional trace settings
	);

	if (bIsHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("We hit something"));
		// start to end, green, will lines always stay on, depth priority, thickness of line
		DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Green, false, 5.f, ECC_WorldStatic, 1.f);
		
		if (Hit.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit actor name %s"), *Hit.GetActor()->GetName());
			UE_LOG(LogTemp, Warning, TEXT("Hit actor distance %s"), *FString::SanitizeFloat(Hit.Distance));
			DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(10.f, 10.f, 10.f), FColor::Red, false, 5.f, ECC_WorldStatic, 5.f);

			UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Hit.GetActor()->GetRootComponent());
			if (PrimComp && PrimComp->IsSimulatingPhysics())
			{
				PrimComp->AddImpulseAtLocation(CameraRotation.Vector() * 1000.f * PrimComp->GetMass(), Hit.ImpactPoint);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing was hit"));
		// start to end, purple, will lines always stay on, depth priority, thickness of line
		DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Purple, false, 5.f, ECC_WorldStatic, 1.f);
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
		UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "Arms", 0.0f);
		}
	}
}

void AFPSCharacter::Throw()
{
	ServerThrow();

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

void AFPSCharacter::ServerThrow_Implementation()
{
	if (ThrowableClass)
	{
		//To Spawn outside player collision box
		float ProjectileSpawnOffset = 30.0f;

		FVector PlayerLocation = GetActorLocation();
		FRotator PlayerRotation = GetFirstPersonCameraComponent()->GetComponentRotation();

		FVector SpawnLocation = PlayerLocation + (PlayerRotation.Vector() * ProjectileSpawnOffset);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ActorSpawnParams.Instigator = this;

		// spawn the projectile at Character Location
		GetWorld()->SpawnActor<AFPSProjectile>(ThrowableClass, SpawnLocation, PlayerRotation, ActorSpawnParams);
	}
}

bool AFPSCharacter::ServerThrow_Validate()
{
	return true;
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

void AFPSCharacter::Die()
{
	/* AFPSWeapon* FPSWeapon = Cast<AFPSWeapon>(Weapon);
	if (FPSWeapon)
	{
		FPSWeapon->BoxComponent->SetSimulatePhysics(true);
	} */

	Destroy();
}

void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSCharacter, bIsCarryingObjective);
	//DOREPLIFETIME_CONDITION(AFPSCharacter, bIsCarryingObjective, COND_OwnerOnly);
}