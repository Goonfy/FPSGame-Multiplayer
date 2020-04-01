// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSGrenade.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class FPSGAME_API AFPSGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, Category= "Components")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* OuterSphereComponent;

	UPROPERTY(EditDefaultsOnly, Category= "Components")
	UStaticMeshComponent* GrenadeMesh;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* GrenadeMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade Effects")
	UParticleSystem* ActivateGrenadeEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade Sound Effects")
	USoundBase* ActivateGrenadeSound;

	UPROPERTY(EditDefaultsOnly, Category= "Grenade Settings")
	float MaxFuzeTime;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float GrenadeRadius;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grenade Settings")
	float GrenadeForceStrength;

	/* Handle to manage the timer */
	FTimerHandle FuzeTimerHandle;

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
 	void OnExplode();
};
