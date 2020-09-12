// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RPGGameModeBase.h"
#include "GameFramework/GameModeBase.h"
#include "RPGGameMode.generated.h"

UCLASS(minimalapi)
class ARPGGameMode : public ARPGGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PawnKilled(APawn* PawnKilled) override;
private:
	void EndGame(bool isPlayerWinner);
};
