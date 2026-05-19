// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveToAnimNotifyState.h"
#include <ProjectDemon/MontageMovementInterface.h>
#include <ProjectDemon/DemonCharacter.h>
#include <Kismet/GameplayStatics.h>

void UMoveToAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp->GetOwner())
	{
		ADemonCharacter* montageMovementInterface = Cast<ADemonCharacter>(MeshComp->GetOwner());
		if (montageMovementInterface)
		{
			auto currentMontage = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
			montageMovementInterface->Controller->SetIgnoreMoveInput(true);
			MeshComp->GetAnimInstance()->RootMotionMode = ERootMotionMode::IgnoreRootMotion;

			FVector targetLocation = montageMovementInterface->getTargetLocation();
			targetLocation.Z = montageMovementInterface->GetActorLocation().Z;

			FVector targetToCharVect = montageMovementInterface->GetActorLocation() - targetLocation;
			targetToCharVect.Normalize();

			float distOffsetFromTarget = 100;
			auto world = montageMovementInterface->GetWorld();

			/*
			UKismetSystemLibrary::DrawDebugArrow(world, targetLocation, targetLocation + targetToCharVect * distOffsetFromTarget, 20.0, FLinearColor::Blue, 10.0);
			UKismetSystemLibrary::DrawDebugSphere(world, targetLocation + targetToCharVect * distOffsetFromTarget, 20.0, 12, FLinearColor::White, 10.0);
			*/
			targetLocation += targetToCharVect * distOffsetFromTarget;

			FVector actorFowardVect = targetLocation - montageMovementInterface->GetActorLocation();
			actorFowardVect.Normalize();

			

			float dis = FVector::Dist(targetLocation, montageMovementInterface->GetActorLocation());

			FVector maxVect = maxVectValInMontage(currentMontage, MeshComp->GetAnimInstance()->Montage_GetPosition(currentMontage) + TotalDuration);
			rootInitialLoc = maxVectValInMontage(currentMontage, MeshComp->GetAnimInstance()->Montage_GetPosition(currentMontage));

			float dif = FVector::Dist(maxVect, rootInitialLoc);

			float xScale = dis / dif;

			rootMotionVectorScale = FVector(1, xScale, 1);
			AnimCharOriginalLocation = montageMovementInterface->GetActorLocation();
			AnimCharForwardVector = actorFowardVect;
			AnimCharUpVector = UKismetMathLibrary::GetUpVector(actorFowardVect.Rotation());
			AnimCharRightVector = UKismetMathLibrary::GetRightVector(actorFowardVect.Rotation());
			
			FVector targetFwdVect = UKismetMathLibrary::GetForwardVector(FRotator(0, montageMovementInterface->FollowCamera->GetComponentRotation().Yaw, 0));
			FVector rightTargetVect = UKismetMathLibrary::GetRightVector(targetFwdVect.Rotation());

			if (FVector::DotProduct(actorFowardVect, rightTargetVect) > FVector::DotProduct(actorFowardVect, -rightTargetVect))
			{
				rightTargetVect *= -1;
			}

			float AngleInRadians = FMath::Acos(FVector::DotProduct(actorFowardVect, rightTargetVect));
			// Convert to degrees if needed
			float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);

			float forwardVal = dis * FMath::Sin(AngleInRadians);
			float sideVal = dis * FMath::Cos(AngleInRadians);

			/*
			UKismetSystemLibrary::DrawDebugArrow(world, AnimCharOriginalLocation, AnimCharOriginalLocation + targetFwdVect * forwardVal, 12, FLinearColor::Blue, 10);
			UKismetSystemLibrary::DrawDebugArrow(world, AnimCharOriginalLocation, AnimCharOriginalLocation + rightTargetVect * sideVal, 12, FLinearColor::Yellow, 10);
			UKismetSystemLibrary::DrawDebugArrow(world, AnimCharOriginalLocation, AnimCharOriginalLocation + rightTargetVect * sideVal + targetFwdVect * forwardVal, 12, FLinearColor::Red, 10);
			*/
		}
	}
}

void UMoveToAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	ADemonCharacter* montageMovementInterface = Cast<ADemonCharacter>(MeshComp->GetOwner());
	if (montageMovementInterface)
	{
		MeshComp->GetAnimInstance()->RootMotionMode = ERootMotionMode::IgnoreRootMotion;

		if (MeshComp->GetAnimInstance()->GetCurrentActiveMontage())
		{
			auto currentMont = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
			for (auto section : currentMont->CompositeSections)
			{
				if (const UAnimSequence* animSequence = Cast<UAnimSequence>(section.GetLinkedSequence()))
				{
					FTransform OutAtom;
					auto T = MeshComp->GetAnimInstance()->Montage_GetPosition(currentMont);
					FSkeletonPoseBoneIndex BoneIndex(0);
					animSequence->GetBoneTransform(OutAtom, BoneIndex, T, true);

					FVector Offset = OutAtom.GetLocation();

					FVector NewCharacterLocation = AnimCharOriginalLocation +
						Offset.Y * AnimCharForwardVector * rootMotionVectorScale.Y +
						Offset.Z * AnimCharUpVector * rootMotionVectorScale.Z +
						Offset.X * AnimCharRightVector * rootMotionVectorScale.X;

					FVector originalPoint = 
						rootInitialLoc.Y * AnimCharForwardVector * rootMotionVectorScale.Y +
						rootInitialLoc.Z * AnimCharUpVector * rootMotionVectorScale.Z +
						rootInitialLoc.X * AnimCharRightVector * rootMotionVectorScale.X;

					NewCharacterLocation -= originalPoint;

					FVector Velocity = (NewCharacterLocation - montageMovementInterface->GetActorLocation()) / FrameDeltaTime;
					montageMovementInterface->GetCharacterMovement()->Velocity = Velocity;
				}
			}
		}
	}	
}

void UMoveToAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	MeshComp->GetAnimInstance()->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
	ADemonCharacter* montageMovementInterface = Cast<ADemonCharacter>(MeshComp->GetOwner());
	if (montageMovementInterface)
	{
		if (montageMovementInterface->Controller)
		{
			montageMovementInterface->Controller->SetIgnoreMoveInput(false);
		}
		
		montageMovementInterface->Log("End?");
	}
}

FVector UMoveToAnimNotifyState::maxVectValInMontage(UAnimMontage* montage)
{
	float maxX = 0.0;
	float maxY = 0.0;
	float maxZ = 0.0;

	for (auto section : montage->CompositeSections)
	{
		if (const UAnimSequence* animSequence = Cast<UAnimSequence>(section.GetLinkedSequence()))
		{
			for (int frame = 0; frame < animSequence->GetNumberOfSampledKeys(); frame++)
			{
				FTransform OutAtom;
				auto T = animSequence->GetTimeAtFrame(frame);
				FSkeletonPoseBoneIndex BoneIndex(0);
				animSequence->GetBoneTransform(OutAtom, BoneIndex, T, true);
				FVector loc = OutAtom.GetLocation();

				if (loc.X > maxX)
				{
					maxX = loc.X;
				}

				if (loc.Y > maxY)
				{
					maxY = loc.Y;
				}

				if (loc.Z > maxZ)
				{
					maxZ = loc.Z;
				}
			}
		}
	}
	return FVector(maxX, maxY, maxZ);
}
FVector UMoveToAnimNotifyState::maxVectValInMontage(UAnimMontage* montage, float time)
{
	float maxX = 0.0;
	float maxY = 0.0;
	float maxZ = 0.0;

	for (auto section : montage->CompositeSections)
	{
		if (const UAnimSequence* animSequence = Cast<UAnimSequence>(section.GetLinkedSequence()))
		{
			animSequence->GetTimeAtFrame(time);
			FTransform OutAtom;
			FSkeletonPoseBoneIndex BoneIndex(0);
			animSequence->GetBoneTransform(OutAtom, BoneIndex, time, true);
			return OutAtom.GetLocation();

		}
	}
	return FVector(maxX, maxY, maxZ);
}

