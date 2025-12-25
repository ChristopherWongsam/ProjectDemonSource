// Fill out your copyright notice in the Description page of Project Settings.


#include "MontagePlayRateAnimNotifyState.h"
#include <Animation/AnimNotifyLibrary.h>

void UMontagePlayRateAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration,EventReference);
	if (MeshComp && MeshComp->GetAnimInstance())
	{
		UAnimMontage* montage = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
		if (montage)
		{
			MeshComp->GetAnimInstance()->Montage_SetPlayRate(montage, startPlayRate);
			refMontage = montage;
		}
	}
}

void UMontagePlayRateAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetAnimInstance() && UAnimNotifyLibrary::NotifyStateReachedEnd(EventReference))
	{
		UAnimMontage* montage = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
		if (montage && montage == refMontage)
		{
			MeshComp->GetAnimInstance()->Montage_SetPlayRate(montage, endPlayRate);
		}
	}
}
