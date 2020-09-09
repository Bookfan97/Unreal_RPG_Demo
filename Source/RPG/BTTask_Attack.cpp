// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"


#include "AIController.h"
#include "RPGCharacter.h"

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = "Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    if (OwnerComp.GetAIOwner() == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("No AI owner"));
        return EBTNodeResult::Failed;
    }

    ARPGCharacter* Character = Cast<ARPGCharacter>(OwnerComp.GetAIOwner()->GetPawn());
    if (Character == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("No AI owner"));
        return EBTNodeResult::Failed;
    }
   
    Character->Attack();
    return EBTNodeResult::Succeeded;
}
