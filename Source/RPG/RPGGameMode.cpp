// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGGameMode.h"

#include "EngineUtils.h"
#include "MagicAIController.h"
#include "RPGCharacter.h"
#include "Blueprint/UserWidget.h"
#include "RPGGameModeBase.h"
#include "RPGPlayerController.h"
#include "UObject/ConstructorHelpers.h"

void ARPGGameMode::PawnKilled(APawn* PawnKilled)
{
	Super::PawnKilled(PawnKilled);
	UE_LOG(LogTemp, Warning, TEXT("Pawn killed"));
	ARPGPlayerController* PlayerController = Cast<ARPGPlayerController>(PawnKilled->GetController());
	if (PlayerController != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is dead"));
		EndGame(false);
	}
	for (AMagicAIController* AIController : TActorRange<AMagicAIController>(GetWorld()))
	{
		if (!AIController->IsDead())
		{
			UE_LOG(LogTemp, Warning, TEXT("AI is still alive"));
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("AI are dead"));
	EndGame(true);
}

void ARPGGameMode::EndGame(bool isPlayerWinner)
{
	for (AController* Controller : TActorRange<AController>(GetWorld()))
	{
		bool bIsWinner = Controller->IsPlayerController() == isPlayerWinner;
		Controller->GameHasEnded(Controller->GetPawn(), bIsWinner);
		UE_LOG(LogTemp, Warning, TEXT("Ended game, winner is"), bIsWinner);
	}
}