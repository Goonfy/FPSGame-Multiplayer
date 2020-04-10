// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

UCLASS()
class FPSGAME_API AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSGameMode();

	void CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void OnMissionCompleted(APawn* InstigatorPawn, bool bMissionSuccess);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Spectating")
	TSubclassOf<AActor> SpectatingViewpointClass;
};



