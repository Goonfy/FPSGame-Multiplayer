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
class AActor;


UCLASS()
class FPSGAME_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSCharacter();

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	bool bIsCarryingObjective;

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1PComponent; }

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return CameraComponent; }

protected:
	/** Pawn mesh: 1st person view  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	USkeletalMeshComponent* Mesh1PComponent;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* GunMeshComponent;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComponent;

	UPROPERTY(EditDefaultsOnly, Category="Throwable")
	TSubclassOf<AActor> ThrowableClass;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	USoundBase* ThrowSound;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	UAnimSequenceBase* ThrowAnimation;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	UAnimSequenceBase* FireAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UPawnNoiseEmitterComponent* NoiseEmitterComponent;
	
	void Fire();

	void Throw();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
};

