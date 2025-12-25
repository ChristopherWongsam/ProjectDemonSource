// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMetaData.h"
#include "RootScaleMetaData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API URootScaleMetaData : public UAnimMetaData
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Root scale of animation"))
	float RootScale = 1.0;
public:
	float getAnimRootMotionScale() const { return RootScale; }
};
