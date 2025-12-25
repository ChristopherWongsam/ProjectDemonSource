// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy.h"
#include <Kismet/GameplayStatics.h>

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	Enemy = Cast<AEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}
	//return;
	if (!EnemyBT)
	{
		return;
	}
	RunBehaviorTree(EnemyBT);
	MyPlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	GetBlackboardComponent()->SetValueAsObject(TEXT("AttackTarget"), MyPlayerCharacter);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!MyPlayerCharacter)
	{
		MyPlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		return;
	}
	if (!Enemy)
	{
		Enemy = Cast<AEnemy>(GetPawn());
		return;
	}
	if (!EnemyBT)
	{
		return;
	}
	//UKismetSystemLibrary::PrintString(this, "Running Enemy AI Tick");
	SetFocus(MyPlayerCharacter);
	float distanceFromPlayer = Enemy->GetDistanceTo(MyPlayerCharacter);
	GetBlackboardComponent()->SetValueAsFloat(TEXT("DistanceFromPlayer"), distanceFromPlayer);
}
