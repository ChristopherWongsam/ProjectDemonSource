// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class PROJECTDEMON_API MontageMovementInterface
{
public:
	MontageMovementInterface();
	~MontageMovementInterface();
	FVector targetActorLocation = FVector::ZeroVector;
	FVector getTargetLocation() const { return targetActorLocation; }
	void setTargetLocation(FVector TargetActorLocation) { this->targetActorLocation = TargetActorLocation; }
	void resetTargetLocation() { this->targetActorLocation = FVector::ZeroVector; }
};
