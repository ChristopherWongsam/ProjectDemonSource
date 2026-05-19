// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectDemon/EnemyBTTask/BTTask_EnemyBase.h"
#include "BTTask_EnemyMovemementSpeed.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API UBTTask_EnemyMovemementSpeed : public UBTTask_EnemyBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	float walkSpeed = 600.0;
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
