// Fill out your copyright notice in the Description page of Project Settings.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FPSHUD.generated.h"

class UTexture2D;

UCLASS()
class FPSGAME_API AFPSHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFPSHUD();

protected:
	/** Crosshair asset pointer */
	UPROPERTY(EditDefaultsOnly)
    UTexture2D* CrosshairTexture;

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;
};

