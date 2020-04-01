// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSLaunchPad.generated.h"

class UBoxComponent;

UCLASS()
class FPSGAME_API AFPSLaunchPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSLaunchPad();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* AreaBox;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* LaunchPadBase;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ArrowPlane;

	UPROPERTY(EditDefaultsOnly, Category = "LaunchPad")
	UParticleSystem* ActivateLaunchPadEffect;

	UPROPERTY(EditDefaultsOnly, Category = "LaunchPad")
	USoundBase* ActivateLaunchPadSound;

	UPROPERTY(EditInstanceOnly, Category = "LaunchPad")
	float LaunchStrenght;

	UPROPERTY(EditInstanceOnly, Category = "LaunchPad")
	float LaunchPitchAngle;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void PlayEffects();

	void PlaySounds();
};
