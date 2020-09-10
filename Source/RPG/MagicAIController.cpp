// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicAIController.h"
#include "RPGCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void AMagicAIController::BeginPlay()
{
    Super::BeginPlay();
    Super::BeginPlay();
    if (AIBehavior != nullptr)
    {
        RunBehaviorTree(AIBehavior);
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        GetBlackboardComponent()->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());
        GetBlackboardComponent()->SetValueAsVector(TEXT("StartLocation"), GetPawn()->GetActorLocation());
    }
}

void AMagicAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    GetBlackboardComponent()->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());
}

bool AMagicAIController::IsDead() const
{
    ARPGCharacter* ControllerCharacter = Cast<ARPGCharacter>(GetPawn());
    if (ControllerCharacter != nullptr)
    {
        return  ControllerCharacter->IsDead();
    }
    return true;
}
