// Fill out your copyright notice in the Description page of Project Settings.


#include "RotateToPlayerAnimNotifyState.h"
#include <ProjectDemon/Enemy.h>


void URotateToPlayerAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	AEnemy* enemy = Cast<AEnemy>(MeshComp->GetOwner());
	if (enemy)
	{
		enemy->setEnableCharacterToTargetRotation(true);
	}
}

void URotateToPlayerAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	AEnemy* enemy = Cast<AEnemy>(MeshComp->GetOwner());
	if (enemy)
	{
		enemy->setEnableCharacterToTargetRotation(false);
	}
}

void URotateToPlayerAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	AEnemy* enemy = Cast<AEnemy>(MeshComp->GetOwner());
	if (enemy)
	{
		enemy->setEnableCharacterToTargetRotation(true);
		enemy->updateCharacterRotationToTarget(FrameDeltaTime, 10.0);
	}
}
