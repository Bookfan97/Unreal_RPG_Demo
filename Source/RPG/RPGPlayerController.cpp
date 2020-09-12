// Fill out your copyright notice in the Description page of Project Settings.

#include "RPGPlayerController.h"

#include "Blueprint/UserWidget.h"

void ARPGPlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
    Super::GameHasEnded(EndGameFocus, bIsWinner);
    if (bIsWinner)
    {
        UE_LOG(LogTemp, Warning, TEXT("Winner"), bIsWinner);
        UUserWidget* WinScreen = CreateWidget(this, WinScreenClass);
        if (WinScreen != nullptr)
        {
            WinScreen->AddToViewport();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Loser"), bIsWinner);
        UUserWidget* LoseScreen = CreateWidget(this, LoseScreenClass);
        if (LoseScreen != nullptr)
        {
            LoseScreen->AddToViewport();
        }
    }
    GetWorldTimerManager().SetTimer(RestartTimer, this, &APlayerController::RestartLevel, RestartDelay);
}