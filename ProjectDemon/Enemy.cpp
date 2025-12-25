// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "AIController.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>
#include <Runtime/Engine/Private/InterpolateComponentToAction.h>
#include "DemonCharacter.h"
#include "AnimNotifyState/AttackAnimNotifyState.h"


void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->GetOverlayMaterial();
	if (GetMesh()->GetOverlayMaterial())
	{
		currMaterial = GetMesh()->GetOverlayMaterial();
		GetMesh()->SetOverlayMaterial(nullptr);
	}
	else
	{
		PrintLog("Enemy Overlay material not set");
	}
	EnemyController = Cast<AEnemyAIController>(GetController());
	if (EnemyController)
	{
		PrintLog("Enemy controller set.");
	}
	else
	{
		PrintLog("Enemy controller not set!!!");
		return;
	}
	MyPlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (MyPlayerCharacter)
	{
		PrintLog("Enemy found main character!!!");
	}
	else
	{
		return;
	}
	EnemyAnimInstance = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());
}
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	StartHitbox(DeltaTime, true,true);
	//updateCharacterRotationToTarget(DeltaTime);
}

void AEnemy::LogScreen(FString log, FLinearColor color)
{
	Super::LogScreen(log, FLinearColor::Red);
}
void AEnemy::Log(FString log, bool printToScreen)
{
	Super::Log(log, printToScreen);
}
void AEnemy::enableOutline(bool enableOutline)
{
	GetMesh()->GetOverlayMaterial();
	if (enableOutline)
	{
		if (currMaterial)
		{
			GetMesh()->SetOverlayMaterial(currMaterial);
		}
		else
		{
			Log("Enemy has no current material", false);
		}
	}
	else
	{
		GetMesh()->SetOverlayMaterial(nullptr);
	}
}

bool AEnemy::enemyIsHighlighted()
{
	return GetMesh()->GetOverlayMaterial() != nullptr;
}

float AEnemy::Attack()
{
	LogScreen("Attacking Player", FLinearColor::Red);
	return PlayMontage(LightAttackMontage);
}

void AEnemy::StartHitbox(float deltaTime, bool bEnableRightPunch, bool enableDebug)
{
	try
	{
		FName SocketName;
		TArray<FString> AttackTags;
		if (!EnemyAnimInstance)
		{
			return;
		}
		TArray<FAnimNotifyEvent> fAnimNotifyEvents = EnemyAnimInstance->ActiveAnimNotifyState;

		if (fAnimNotifyEvents.Num() == 0)
		{
			actorsHit.Empty();
		}
		
		bool isFound = false;

		for (FAnimNotifyEvent fAnimNotifyEvent : fAnimNotifyEvents)
		{
			FString notifyName = fAnimNotifyEvent.NotifyName.ToString();

			if (UAttackAnimNotifyState* attackAnimNotifyState = Cast<UAttackAnimNotifyState>(fAnimNotifyEvent.NotifyStateClass))
			{
				EAttackType attackType = attackAnimNotifyState->getAttackType();
				switch (attackType)
				{
				case EAttackType::EAT_LeftPunch:
					SocketName = LeftHandSocketName;
					break;
				case EAttackType::EAT_RightPunch:
					SocketName = RightHandSocketName;
					break;
				case EAttackType::EAT_LeftKick:
					SocketName = LeftFootSocketName;
					break;
				case EAttackType::EAT_RightKick:
					SocketName = RightFootSocketName;
					break;
				case EAttackType::EAT_RightWeapon:
					SwordHitbox(this->RightWeaponSocketInitialPoint, this->RightWeaponSocketFinalPoint);
					break;
				default:
					break;
				}
				isFound = attackType != EAttackType::EAT_None;
				if (!SocketName.IsNone())
				{
					AttackHitbox(SocketName,true);
				}
			}
		}

		if (!isFound)
		{
			actorsHit.Empty();
		}

	}
	catch (const std::exception& e)
	{
		Log(e.what());
	}
}

void AEnemy::updateCharacterRotationToTarget(float deltaTime, float interpSpeed, FVector targetLocation, bool enableDebug)
{
	if (!MyPlayerCharacter)
	{
		return;
	}
	if(!getEnableCharacterTargetRoataion())
	{ 
		return;
	}
	float newYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), MyPlayerCharacter->GetActorLocation()).Yaw;
	if (!targetLocation.Equals(FVector::ZeroVector))
	{
		newYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), targetLocation).Yaw;
	}
	FRotator newRot(0, newYaw, 0);
	newRot = UKismetMathLibrary::RInterpTo_Constant(GetActorRotation(), newRot, deltaTime, interpSpeed);
	SetActorRotation(newRot);
	
}

void AEnemy::AttackHitbox(FName SocketName, bool bEnableDebug)
{
	FVector AttackPoint = GetActorLocation();
	float zLoc = AttackPoint.Z;
	AttackPoint = GetMesh()->GetSocketLocation(SocketName);
	FVector AttackCenterPoint = AttackPoint;
	AttackCenterPoint.Z = GetActorLocation().Z;

	FVector AttackVector = AttackCenterPoint - GetActorLocation();

	float AttackVectorSize = GetCapsuleComponent()->GetScaledCapsuleRadius();
	float AttackCenterDist = AttackVectorSize + GetCapsuleComponent()->GetScaledCapsuleRadius() + 100;

	AttackVector.Normalize();
	auto StartPoint = GetActorLocation();
	StartPoint.Z = AttackPoint.Z;
	auto midPoint = AttackCenterDist * AttackVector + GetActorLocation();

	auto start = midPoint;
	start.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	auto end = midPoint;
	end.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	auto radius = 25.0;

	FHitResult hitResult;

	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	bool didHit = UKismetSystemLibrary::SphereTraceSingleForObjects(this, AttackPoint, AttackPoint, radius,
		traceObjectTypes, false, actorsToIgnore, bEnableDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, hitResult, true);
	if (didHit)
	{
		if (ADemonCharacter* enemy = Cast<ADemonCharacter>(hitResult.GetActor()))
		{
			if (!actorsHit.Contains(enemy))
			{
				actorsHit.Add(enemy);
				enemy->HitReact(this);
				Log("Main Character hit");
				if (bEnableDebug)
				{
					DrawDebugSphere(GetWorld(), hitResult.Location, 25, 12, FColor::Blue, false, 2.0);
				}
			}
		}
	}
}
void AEnemy::SwordHitbox(FName SocketInitName, FName SocketFinalName)
{
	FVector AttackInitPoint = GetMesh()->GetSocketLocation(SocketInitName);
	FVector AttackFinalPoint = GetMesh()->GetSocketLocation(SocketFinalName);

	auto radius = 50;

	FHitResult hitResult;

	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	bool didHit = UKismetSystemLibrary::SphereTraceSingleForObjects(this, AttackInitPoint, AttackFinalPoint, radius, traceObjectTypes, false, actorsToIgnore, EDrawDebugTrace::ForOneFrame, hitResult, true);
	if (didHit)
	{
		if (ADemonCharacter* enemy = Cast<ADemonCharacter>(hitResult.GetActor()))
		{
			if (!actorsHit.Contains(enemy))
			{
				actorsHit.Add(enemy);
				enemy->HitReact(this);
				Log("Main Character hit");
				DrawDebugSphere(GetWorld(), hitResult.Location, 25, 12, FColor::Blue, false, 2.0);
			}
		}
	}
}
float AEnemy::HitReact(AActor* sender)
{
	int index = 0;
	if (HitReactMontageArray.Num() == 0)
	{
		Log("Enemy hit react montage array empty");
		return 0.0;
	}
	if (GetCurrentMontage())
	{
		StopAnimMontage(GetCurrentMontage());
	}

	auto yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), sender->GetActorLocation()).Yaw;

	auto newRot = GetActorRotation();
	newRot.Yaw = yaw;
	SetActorRotation(newRot);

	auto vector = GetActorLocation() - sender->GetActorLocation();
	FName sectionName = "Default";
	vector.Normalize();
	vector *= -1;

	if (FVector::DotProduct(GetActorForwardVector(), vector) < FVector::DotProduct(-GetActorForwardVector(), vector))
	{
		//sectionName = "HitBack";
	}

	auto hitReactMontage = HitReactMontageArray[index];
	float T = PlayMontage(hitReactMontage,sectionName);
	FOnMontageEnded BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &AEnemy::HitReactEnd);
	GetMesh()->GetAnimInstance()->Montage_SetBlendingOutDelegate(BlendOutDelegate, hitReactMontage);
	
	
	return T;
}
void AEnemy::HitReactEnd(UAnimMontage* animMontage, bool bInterrupted)
{
	if (bInterrupted)
	{
		bEnableMirrorAnimation = !bEnableMirrorAnimation;

	}
	else
	{
		Log("Enemy hit react end");
		bEnableMirrorAnimation = false;

	}
}



void AEnemy::ChasePlayer()
{
	Log("Enemy Chasing Player");
	EnemyController->MoveToActor(MyPlayerCharacter, AcceptableAttackRange);
	Delay(0.2, "ChasePlayer");
}
bool AEnemy::GetIsAttackAnimationPlaying()
{
	if (!GetCurrentMontage())
	{
		return false;
	}
	return GetCurrentMontage() == LightAttackMontage;
}