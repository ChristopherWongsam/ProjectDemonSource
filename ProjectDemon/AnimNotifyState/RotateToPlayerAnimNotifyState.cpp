// Fill out your copyright notice in the Description page of Project Settings.


#include "RotateToPlayerAnimNotifyState.h"
#include <ProjectDemon/DemonCharacter.h>
#include <Kismet/GameplayStatics.h>


void URotateToPlayerAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp->GetOwner() && MeshComp->GetAnimInstance() && MeshComp->GetAnimInstance()->GetCurrentActiveMontage())
	{
		ADemonCharacter* montageMovementInterface = Cast<ADemonCharacter>(MeshComp->GetOwner());
		
		if (montageMovementInterface)
		{
			montageMovementInterface->Controller->SetIgnoreMoveInput(true);
			auto currentMontage = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
			MeshComp->GetAnimInstance()->RootMotionMode = ERootMotionMode::IgnoreRootMotion;

			FRotator actorRot = UKismetMathLibrary::FindLookAtRotation(montageMovementInterface->GetActorLocation(), montageMovementInterface->getTargetLocation());

			targetRot = FRotator(0,actorRot.Yaw,0);
			initialRot = FRotator(0,montageMovementInterface->GetActorRotation().Yaw,0);

			float currentTime = MeshComp->GetAnimInstance()->Montage_GetPosition(currentMontage);
			
			int currentTimeFrame = currentMontage->GetFrameAtTime(currentTime);
			int durationEndTimeFrame = currentMontage->GetFrameAtTime(TotalDuration);

			initTime = currentTime;
			duration = TotalDuration + initTime;
		}
	}
}
void URotateToPlayerAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (MeshComp->GetOwner())
	{
		ADemonCharacter* montageMovementInterface = Cast<ADemonCharacter>(MeshComp->GetOwner());
		MeshComp->GetAnimInstance()->RootMotionMode = ERootMotionMode::IgnoreRootMotion;
		if (montageMovementInterface && MeshComp->GetAnimInstance() && MeshComp->GetAnimInstance()->GetCurrentActiveMontage())
		{
			auto currentMontage = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
			float currentTime = MeshComp->GetAnimInstance()->Montage_GetPosition(currentMontage);

			if (currentTime > currentMontage->GetDefaultBlendInTime())
			{
				float ratio = (currentTime - initTime) / (duration - initTime);
				if (duration < initTime)
				{
					ratio = (currentTime - initTime) / (duration);
				}

				montageMovementInterface->Log("The ratio for rotation is: " + FString::SanitizeFloat(ratio));
				FRotator newYaw = UKismetMathLibrary::RLerp(initialRot, targetRot, ratio, true);
				montageMovementInterface->SetActorRotation(newYaw);
			}
		}

	}
}
void URotateToPlayerAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	MeshComp->GetAnimInstance()->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;

	ADemonCharacter* montageMovementInterface = Cast<ADemonCharacter>(MeshComp->GetOwner());
	if (montageMovementInterface && montageMovementInterface->Controller)
	{
		montageMovementInterface->Controller->SetIgnoreMoveInput(false);
		montageMovementInterface->Log("End?");
	}
}


