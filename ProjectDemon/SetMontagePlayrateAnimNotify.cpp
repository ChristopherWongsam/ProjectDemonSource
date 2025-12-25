// Fill out your copyright notice in the Description page of Project Settings.


#include "SetMontagePlayrateAnimNotify.h"

void USetMontagePlayrateAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetAnimInstance())
	{
		UAnimMontage* montage = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
		if (montage)
		{
			MeshComp->GetAnimInstance()->Montage_SetPlayRate(montage, playRate);
		}
	}
}
