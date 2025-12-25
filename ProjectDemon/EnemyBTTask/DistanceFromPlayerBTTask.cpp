// Fill out your copyright notice in the Description page of Project Settings.


#include "DistanceFromPlayerBTTask.h"

EBTNodeResult::Type UDistanceFromPlayerBTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	float distanceFromPlayer = GetEnemy()->GetDistanceTo(GetPlayerCharacter());
	OwnerComp.GetBlackboardComponent()->SetValueAsFloat(TEXT("DistanceFromPlayer"), distanceFromPlayer);
	return EBTNodeResult::Type();
}
