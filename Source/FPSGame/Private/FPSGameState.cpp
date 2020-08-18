// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameState.h"
#include "EngineUtils.h"
#include "FPSPlayerController.h"

void AFPSGameState::MulticastOnMissionComplete_Implementation(APawn* InstigatorPawn, bool bMissionSuccess)
{
    for (TActorIterator<AFPSPlayerController> PC(GetWorld()); PC; ++PC)
    {
        if (PC->IsLocalController())
        {
            PC->OnMissionCompleted(InstigatorPawn, bMissionSuccess);

            APawn* Pawn = PC->GetPawn();
            if (Pawn)
            {
                Pawn->DisableInput(nullptr);
            }
        }
    }
}