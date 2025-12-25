// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMetaData.h"
#include "AttackRangeMetaData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API UAttackRangeMetaData : public UAnimMetaData
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Root scale of animation"))
	float AttackRange = 1.0;
public:
	float getAttackRange() const { return AttackRange; }
};
