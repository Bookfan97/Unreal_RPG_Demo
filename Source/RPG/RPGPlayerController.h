// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RPGPlayerController.generated.h"

/**
*
*/
UCLASS()
class RPG_API ARPGPlayerController : public APlayerController
{
    GENERATED_BODY()
    public:
    virtual void GameHasEnded(AActor* EndGameFocus = nullptr, bool bIsWinner = false) override;
    private:
    UPROPERTY(EditAnywhere)
    TSubclassOf<class UUserWidget> LoseScreenClass;
    UPROPERTY(EditAnywhere)
    TSubclassOf<class UUserWidget> WinScreenClass;
    UPROPERTY(EditAnywhere)
    float RestartDelay = 5;
    FTimerHandle RestartTimer;
};
