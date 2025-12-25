// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BTTask_EnemyBase.h"
#include "DistanceFromPlayerBTTask.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API UDistanceFromPlayerBTTask : public UBTTask_EnemyBase
{
	GENERATED_BODY()
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
