// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBlackHoleGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

// Sets default values
AFPSBlackHoleGrenade::AFPSBlackHoleGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GrenadeRadius = 1500;
	GrenadeForceStrength = -3000;

	MaxFuzeTime = 5;
	BlackHoleLifeSpan = 10;

	IsExploding = false;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSBlackHoleGrenade::OnHit);	// set up a notification for when this component hits something blocking
	
	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	
	// Set as root component
	RootComponent = CollisionComp;

	OuterSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OuterSphereComp"));
	OuterSphereComponent->InitSphereRadius(GrenadeRadius);
	OuterSphereComponent->SetCollisionProfileName("OverlapAll");
	OuterSphereComponent->SetupAttachment(RootComponent);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	GrenadeMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	GrenadeMovement->UpdatedComponent = CollisionComp;
	GrenadeMovement->InitialSpeed = 1500.0f;
	GrenadeMovement->MaxSpeed = 1500.0f;
	GrenadeMovement->bRotationFollowsVelocity = true;
	GrenadeMovement->bShouldBounce = true;

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFPSBlackHoleGrenade::BeginPlay()
{
	Super::BeginPlay();
	
	/* Activate the fuze to explode the bomb after several seconds */
	GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle, this, &AFPSBlackHoleGrenade::OnExplode, MaxFuzeTime, false);
}

// Called every frame
void AFPSBlackHoleGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsExploding)
	{
		TArray<AActor*> OverlappingActors;
		OuterSphereComponent->GetOverlappingActors(OverlappingActors);
		for (int32 i = 0; i < OverlappingActors.Num(); i++)
		{
			UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(OverlappingActors[i]->GetRootComponent());
			if (PrimComp && PrimComp->IsSimulatingPhysics())
			{
				UE_LOG(LogTemp, Log, TEXT("Environment items overlapped with blackhole zone!!"));

				// the component we are looking for! It needs to be simulating in order to apply forces.

				PrimComp->AddRadialForce(Location, GrenadeRadius, GrenadeForceStrength, ERadialImpulseFalloff::RIF_Constant, true);
			}

			ACharacter* Character = Cast<ACharacter>(OverlappingActors[i]);
			if (Character)
			{
				UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(Character->GetMovementComponent());
				if (CharacterComp)
				{
					UE_LOG(LogTemp, Log, TEXT("Character overlapped with blackhole zone!!"));

					//CharacterComp->StopMovementImmediately();
					//CharacterComp->DisableMovement();
					//CharacterComp->SetComponentTickEnabled(false);

					CharacterComp->AddRadialForce(Location, GrenadeRadius, GrenadeForceStrength * CharacterComp->Mass, ERadialImpulseFalloff::RIF_Constant);
				}
			}
		}
	}
}

void AFPSBlackHoleGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}
}

void AFPSBlackHoleGrenade::OnExplode()
{
	Location = GetActorLocation();
	Location.Z += 500;

	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(GrenadeRadius);

	DrawDebugSphere(GetWorld(), Location, MyColSphere.GetSphereRadius(), 50, FColor::Red, false, BlackHoleLifeSpan, 0, 1);

	GrenadeMesh->DestroyComponent();
	GrenadeMovement->DestroyComponent();
	
	IsExploding = true;

	if (ActivateGrenadeEffect)
		UGameplayStatics::SpawnEmitterAtLocation(this, ActivateGrenadeEffect, GetActorLocation());
	if (ActivateGrenadeSound)
		UGameplayStatics::PlaySoundAtLocation(this, ActivateGrenadeSound, GetActorLocation());

	// Clear ALL timers that belong to this (Actor) instance.
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle, this, &AFPSBlackHoleGrenade::EndBlackHole, BlackHoleLifeSpan, false);
}

void AFPSBlackHoleGrenade::EndBlackHole()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Destroy();
}