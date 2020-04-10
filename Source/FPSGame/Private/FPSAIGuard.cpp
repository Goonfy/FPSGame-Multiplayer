// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "FPSGameMode.h"
#include "Engine/World.h"
#include "AIController.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnNoiseHeard);

	GuardState = EAIState::Idle;

	PatrolPointNumber = 0;
	bIsSuspicious = false;
}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalRotation = GetActorRotation();

	MoveToNextPatrolPoint();
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPatrolPoint)
	{
		if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle && !bIsSuspicious)
		{
			MoveToNextPatrolPoint();
		}
	}
}

void AFPSAIGuard::OnPawnSeen(APawn* SeenPawn)
{
	if (SeenPawn)
	{
		DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Red, false, 10.0f, 0.0f, 1.0f);	
	}

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CompleteMission(SeenPawn, false);
	}

	SetGuardState(EAIState::Alerted);

	// Stop Movement
	if (AIController)
	{
		AIController->StopMovement();
	}

	bIsSuspicious = true;
}

void AFPSAIGuard::OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	if (GuardState == EAIState::Alerted)
	{
		return;
	}

	DrawDebugSphere(GetWorld(), Location, 32.0f, 12, FColor::Green, false, 10.0f, 0.0f, 1.0f);

	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;

	SetActorRotation(NewLookAt);

	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 5.0f);

	SetGuardState(EAIState::Suspicious);

	// Stop Movement
	if (AIController)
	{
		AIController->StopMovement();
	}

	bIsSuspicious = true;
}

void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alerted)
	{
		return;
	}

	SetActorRotation(OriginalRotation);

	SetGuardState(EAIState::Idle);

	MoveToNextPatrolPoint();

	bIsSuspicious = false;
}

void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	if (GuardState == NewState)
	{
		return;
	}

	GuardState = NewState;

	OnStateChanged(GuardState);
}

void AFPSAIGuard::MoveToNextPatrolPoint()
{
	for(auto& PatrolPoint : PatrolPoints)
	{
		if (PatrolPoint != CurrentPatrolPoint)
		{
			CurrentPatrolPoint = PatrolPoint;

			AIController = Cast<AAIController>(GetController());
			if (AIController)
			{
				AIController->MoveToActor(CurrentPatrolPoint);
			}

			break;
		}

		/* if (PatrolPointNumber >= PatrolPoints.Num() - 1)
		{
			PatrolPointNumber = 0;
		} */
	}
}