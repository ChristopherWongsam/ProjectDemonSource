// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetSystemLibrary.h>
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectDemon/EnemyBTTask/BTTask_EnemyMovemementSpeed.h"

EBTNodeResult::Type UBTTask_EnemyMovemementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	if (GetEnemy())
	{
		GetEnemy()->GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
	}
	return EBTNodeResult::Type();
}
