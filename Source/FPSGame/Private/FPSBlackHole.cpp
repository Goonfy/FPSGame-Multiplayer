// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBlackHole.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

// Sets default values
AFPSBlackHole::AFPSBlackHole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BlackHoleRadius = 1000;
	BlackHoleForceStrength = -2000;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = MeshComp;

	InnerSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("InnerSphereComp"));
	InnerSphereComponent->SetSphereRadius(100);
	InnerSphereComponent->SetupAttachment(MeshComp);

	// Bind to Event
	InnerSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AFPSBlackHole::OverlapInnerSphere);

	OuterSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OuterSphereComp"));
	OuterSphereComponent->SetSphereRadius(BlackHoleRadius);
	OuterSphereComponent->SetupAttachment(MeshComp);

	// Die after x seconds by default
	InitialLifeSpan = 5.0f;
}

void AFPSBlackHole::OverlapInnerSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		OtherActor->Destroy();
	}
}

// Called when the game starts or when spawned
void AFPSBlackHole::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFPSBlackHole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<AActor*> OverlappingActors;
	OuterSphereComponent->GetOverlappingActors(OverlappingActors);
	for (int32 i = 0; i < OverlappingActors.Num(); i++)
	{
		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(OverlappingActors[i]->GetRootComponent());
		if (PrimComp && PrimComp->IsSimulatingPhysics())
		{
			UE_LOG(LogTemp, Log, TEXT("Environment items overlapped with blackhole zone!!"));

			// the component we are looking for! It needs to be simulating in order to apply forces.

			PrimComp->AddRadialForce(GetActorLocation(), BlackHoleRadius, BlackHoleForceStrength, ERadialImpulseFalloff::RIF_Constant, true);
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

				CharacterComp->AddRadialForce(GetActorLocation(), BlackHoleRadius, BlackHoleForceStrength * CharacterComp->Mass, ERadialImpulseFalloff::RIF_Constant);
			}
		}
	}
}

