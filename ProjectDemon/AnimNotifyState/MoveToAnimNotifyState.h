// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MoveToAnimNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API UMoveToAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	FVector maxVectValInMontage(UAnimMontage* montage);
	FVector maxVectValInMontage(UAnimMontage* montage, float time);
private:

	FVector AnimCharOriginalLocation;
	FVector AnimCharForwardVector;
	FVector AnimCharRightVector;
	FVector AnimCharUpVector;
	FVector rootMotionVectorScale;
	FVector rootInitialLoc;
};
