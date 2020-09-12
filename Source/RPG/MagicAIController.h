// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MagicAIController.generated.h"

/**
*
*/
UCLASS()
class RPG_API AMagicAIController : public AAIController
{
	GENERATED_BODY()
	protected:
	virtual void BeginPlay() override;
	public:
	virtual void Tick(float DeltaSeconds) override;
	bool IsDead() const;
	private:
	/*UPROPERTY(EditAnywhere)
	float AcceptanceRadius = 200;*/
	UPROPERTY(EditAnywhere)
	class UBehaviorTree* AIBehavior;
};
