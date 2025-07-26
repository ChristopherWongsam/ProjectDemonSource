// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_TimedNiagaraEffect.h"
#include "NiagaraEffectNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDEMON_API UNiagaraEffectNotifyState : public UAnimNotifyState_TimedNiagaraEffect
{
	GENERATED_BODY()
protected:
	virtual UFXSystemComponent* SpawnEffect(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const override;
public:
	// Offset from the socket / bone rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NiagaraSystem, meta = (ToolTip = "Scale for the Niagara system"))
	FVector EffectScale = FVector(1.0);
};
