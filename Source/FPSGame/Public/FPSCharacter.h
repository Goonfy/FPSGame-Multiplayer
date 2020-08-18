// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class USoundBase;
class UAnimSequence;
class USpringArmComponent;
class UPawnNoiseEmitterComponent;
class AFPSProjectile;

UCLASS()
class FPSGAME_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Pawn mesh: 1st person view  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	USkeletalMeshComponent* Mesh1PComponent;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* GunMeshComponent;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComponent;

	/* UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<AActor> WeaponClass; */

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	UAnimSequenceBase* FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category="Throwable")
	TSubclassOf<AFPSProjectile> ThrowableClass;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	USoundBase* ThrowSound;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	UAnimSequenceBase* ThrowAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UPawnNoiseEmitterComponent* NoiseEmitterComponent;

	void Throw();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerThrow();
	void ServerThrow_Implementation();
	bool ServerThrow_Validate();

	void Fire();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1PComponent; }

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return CameraComponent; }

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Gameplay")
	bool bIsCarryingObjective;

	void Die();

	virtual void Tick(float DeltaTime) override;
};