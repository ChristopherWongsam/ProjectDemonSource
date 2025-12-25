// Fill out your copyright notice in the Description page of Project Settings.

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraEffectNotifyState.h"

UFXSystemComponent* UNiagaraEffectNotifyState::SpawnEffect(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const
{
	// Only spawn if we've got valid params
	if (ValidateParameters(MeshComp))
	{
		if (UNiagaraComponent* NewComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(Template, MeshComp, SocketName, LocationOffset, RotationOffset, 
			EffectScale,EAttachLocation::KeepRelativeOffset, !bDestroyAtEnd, ENCPoolMethod::None))
		{
			if (bApplyRateScaleAsTimeDilation)
			{
				NewComponent->SetCustomTimeDilation(Animation->RateScale);
			}
			return NewComponent;
		}
	}
	return nullptr;
}
