// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AttackAnimNotifyState.generated.h"

UENUM(BlueprintType)
enum class EAttackType :uint8
{
	EAT_LeftPunch UMETA(DisplayName = "LeftPunch"),
	EAT_RightPunch UMETA(DisplayName = "RightPunch"),
	EAT_LeftKick UMETA(DisplayName = "LeftKick"),
	EAT_RightKick UMETA(DisplayName = "RightKick"),
	EAT_LeftWeapon UMETA(DisplayName = "LeftWeapon"),
	EAT_RightWeapon UMETA(DisplayName = "RightWeapon"),
	EAT_None UMETA(DisplayName = "None")
};
/**
 * 
 */
UCLASS()
class PROJECTDEMON_API UAttackAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
protected:
	// Offset from the socket / bone rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AttackType, meta = (ToolTip = "Attack Type"))
	EAttackType AttackType = EAttackType::EAT_None;
public:
	EAttackType getAttackType() { return AttackType; }
};
