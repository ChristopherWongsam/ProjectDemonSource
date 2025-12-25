// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MontagePlayRateAnimNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API UMontagePlayRateAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	// Offset from the socket / bone rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Play rate when state starts"))
	float startPlayRate = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Play rate when state starts"))
	float endPlayRate = 1.0;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
private:
	UAnimMontage* refMontage;

};
