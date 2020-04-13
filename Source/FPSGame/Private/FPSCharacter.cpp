// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/Actor.h"
#include "FPSAIGuard.h"
#include "FPSWeapon.h"
#include "Components/BoxComponent.h"

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

	NoiseEmitterComponent = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ActorSpawnParams.Owner = this;

	// spawn the Weapon at First Person Mesh
	Weapon = GetWorld()->SpawnActor<AActor>(WeaponClass, GetActorLocation(), GetActorRotation(), ActorSpawnParams);
	if (Weapon)
	{
		Weapon->AttachToComponent(Mesh1PComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	}

	/* // spawn the Weapon at Third Person Mesh
	DummyWeapon = GetWorld()->SpawnActor<AActor>(WeaponClass, GetActorLocation(), GetActorRotation(), ActorSpawnParams);
	if (DummyWeapon)
	{
		DummyWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	} */
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSWeapon::Fire);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &AFPSCharacter::Throw);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AFPSCharacter::Throw()
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
		GetWorld()->SpawnActor<AActor>(ThrowableClass, SpawnLocation, PlayerRotation, ActorSpawnParams);
	
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

void AFPSCharacter::Die()
{
	AFPSWeapon* FPSWeapon = Cast<AFPSWeapon>(Weapon);
	if (FPSWeapon)
	{
		FPSWeapon->BoxComponent->SetSimulatePhysics(true);
	}

	Destroy();
}