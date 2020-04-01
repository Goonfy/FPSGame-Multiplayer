// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSLaunchPad.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// Sets default values
AFPSLaunchPad::AFPSLaunchPad()
{
	AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
	RootComponent = AreaBox;

	LaunchPadBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaunchPadBase"));
	LaunchPadBase->SetupAttachment(RootComponent);

	ArrowPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowPlane"));
	ArrowPlane->SetupAttachment(RootComponent);

	AreaBox->OnComponentBeginOverlap.AddDynamic(this, &AFPSLaunchPad::HandleOverlap);
	
	LaunchStrenght = 1500;
	LaunchPitchAngle = 35.0f;
}

void AFPSLaunchPad::PlayEffects()
{
	UGameplayStatics::SpawnEmitterAtLocation(this, ActivateLaunchPadEffect, GetActorLocation());
}

void AFPSLaunchPad::PlaySounds()
{
	UGameplayStatics::PlaySoundAtLocation(this, ActivateLaunchPadSound, GetActorLocation());
}

void AFPSLaunchPad::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Make rotator with our specified "pitch" and convert to a direction vector * intesify
	FRotator LaunchDirection = GetActorRotation();
	LaunchDirection.Pitch += LaunchPitchAngle;
	FVector LaunchVelocity = LaunchDirection.Vector() * LaunchStrenght;

	ACharacter* OtherCharacter = Cast<ACharacter>(OtherActor);
	if (OtherCharacter)
	{
		OtherCharacter->LaunchCharacter(LaunchVelocity, true, true);

		PlayEffects();
		PlaySounds();

		UE_LOG(LogTemp, Log, TEXT("Character overlapped with launchpad zone!!"));
	}
	else if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulse(LaunchVelocity, NAME_None, true);

		PlayEffects();
		PlaySounds();

		UE_LOG(LogTemp, Log, TEXT("Physics object overlapped with launchpad zone!!"));
	}
}

