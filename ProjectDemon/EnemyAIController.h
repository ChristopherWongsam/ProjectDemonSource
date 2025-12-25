// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	ACharacter* MyPlayerCharacter;
	UPROPERTY(EditAnywhere)
	class UBehaviorTree* EnemyBT;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	class AEnemy* Enemy;

};
