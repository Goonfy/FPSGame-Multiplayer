// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

// Sets default values
AFPSGrenade::AFPSGrenade()
{
	GrenadeRadius = 500.f;
	GrenadeForceStrength = 2000.f;

	MaxFuzeTime = 5.f;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSGrenade::OnHit);	// set up a notification for when this component hits something blocking
	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	// Set as root component
	RootComponent = CollisionComp;

	OuterSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OuterSphereComp"));
	OuterSphereComponent->SetSphereRadius(GrenadeRadius);
	OuterSphereComponent->SetupAttachment(RootComponent);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	GrenadeMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	GrenadeMovement->UpdatedComponent = CollisionComp;

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFPSGrenade::BeginPlay()
{
	Super::BeginPlay();

	/* Activate the fuze to explode the bomb after several seconds */
	GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle, this, &AFPSGrenade::OnExplode, MaxFuzeTime, false);
}

void AFPSGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.f, GetActorLocation());
	}
}

void AFPSGrenade::OnExplode()
{
	DrawDebugSphere(GetWorld(), GetActorLocation(), GrenadeRadius, 50, FColor::Red, false, 1.f, 0.f, 1.f);
	
	TArray<AActor*> OverlappingActors;
	OuterSphereComponent->GetOverlappingActors(OverlappingActors);
	for (int32 i = 0; i < OverlappingActors.Num(); i++)
	{
		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(OverlappingActors[i]->GetRootComponent());
		if (PrimComp && PrimComp->IsSimulatingPhysics())
		{
			UE_LOG(LogTemp, Warning, TEXT("Environment items overlapped with grenade zone!!"));

			// the component we are looking for! It needs to be simulating in order to apply forces.

			PrimComp->AddRadialImpulse(GetActorLocation(), GrenadeRadius, GrenadeForceStrength, ERadialImpulseFalloff::RIF_Constant, true);
		}

		ACharacter* Character = Cast<ACharacter>(OverlappingActors[i]);
		if (Character)
		{
			UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(Character->GetMovementComponent());
			if (CharacterComp)
			{
				UE_LOG(LogTemp, Warning, TEXT("Character overlapped with grenade zone!!"));

				CharacterComp->AddRadialImpulse(GetActorLocation(), GrenadeRadius, GrenadeForceStrength, ERadialImpulseFalloff::RIF_Constant, true);
			}
		}
	}

	if (ActivateGrenadeEffect)
		UGameplayStatics::SpawnEmitterAtLocation(this, ActivateGrenadeEffect, GetActorLocation());
	if (ActivateGrenadeSound)
		UGameplayStatics::PlaySoundAtLocation(this, ActivateGrenadeSound, GetActorLocation());

	// Clear ALL timers that belong to this (Actor) instance.
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Destroy();
}