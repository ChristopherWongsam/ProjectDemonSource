// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SetMontagePlayrateAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API USetMontagePlayrateAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
protected:
	// Offset from the socket / bone rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Attack Type"))
	float playRate = 1.0;
public:
	float getPlayRate() { return playRate; }
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
