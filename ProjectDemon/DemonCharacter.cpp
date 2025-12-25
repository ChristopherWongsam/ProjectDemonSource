// Fill out your copyright notice in the Description page of Project Settings.


#include "DemonCharacter.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/KismetMathLibrary.h>
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Runtime/Engine/Private/InterpolateComponentToAction.h>
#include "MyMainCharacterAnimInstance.h"
#include "UObject/UObjectGlobals.h"
#include "Enemy.h"
#include "CharacterComponent/DynamicCameraComponent.h"
#include "C:/UE_5.4/Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "C:/UE_5.4/Engine/Plugins/Runtime/CableComponent/Source/CableComponent/Classes/CableActor.h"
#include "BezierCurve.h"
#include <CableComponent.h>
#include "AnimNotifyState/AttackAnimNotifyState.h"

ADemonCharacter::ADemonCharacter()
{
	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));
	DynamicCameraComponent = CreateDefaultSubobject<UDynamicCameraComponent>(TEXT("DynamicCameraComponent"));
	DynamicCameraComponent->RegisterComponent();
	CableComponent->SetupAttachment(GetMesh(), "Spine");
}

void ADemonCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	for (int index : MaterialIndexHide)
	{
		GetMesh()->ShowMaterialSection(index, 0, false, 0);
	}
}

void ADemonCharacter::BeginPlay()
{
	Super::BeginPlay();
	MainCharacterAnimInstance = Cast<UMyMainCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (!MainCharacterAnimInstance)
	{
		Log("Unable to cast MainCharacterAnimInstance");
	}
	orignalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
}
void ADemonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Mantle(DeltaTime);
	SoftLock(DeltaTime);
	StartHitbox(DeltaTime);
	Swing(DeltaTime, true);
	UpdateBoost(DeltaTime);
	UpdateWallclimb(DeltaTime);
	UpdateMovementRotation(DeltaTime);
	UpdatePlayerAttack(DeltaTime);
	UpdateHeadLookAtLocation(DeltaTime);
	UpdateDodgeGlide(DeltaTime);
	UpdateDodge(DeltaTime);
	UpdateCableActor(DeltaTime);
}
void ADemonCharacter::DrawInput(float DeltaTime)
{
	auto start = GetActorLocation();
	auto end = GetActorLocation() + GetInputDirection() * 50;
	auto lineThickness = 50.0;
	FHitResult Result;
	UKismetSystemLibrary::DrawDebugArrow(this, start, end, lineThickness, FLinearColor::Yellow);
}

void ADemonCharacter::UpdateSpeed(float DeltaTime)
{

}



void ADemonCharacter::Mantle(float DeltaTime)
{

}
void ADemonCharacter::MantleEnd(UAnimMontage* animMontage, bool bInterrupted)
{
	Log("Mantle end");
	auto newLoc = GetActorLocation();
	newLoc.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	auto playerRot = GetActorRotation();
	GetMovementComponent()->StopMovementImmediately();

	auto StartPoint = GetActorLocation();
	StartPoint.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	auto EndPoint = StartPoint;
	EndPoint.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 10;
	FHitResult Result;
	bool didHit = SphereTrace(StartPoint, EndPoint, 30, ETraceTypeQuery::TraceTypeQuery1, EDrawDebugTrace::None, Result);
	GetCameraBoom()->bDoCollisionTest = true;
	GetController()->SetIgnoreMoveInput(false);
	if (didHit)
	{
		Log("Floor ground hit");
		newLoc = Result.ImpactPoint;
		newLoc.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		playerRot = GetActorRotation();
		GetMovementComponent()->StopMovementImmediately();
		MoveCharacterToRotationAndLocationIninterval(newLoc, playerRot, 0.2);
		ResetCollision();
		return;
	}
	ResetCollision();
}
void ADemonCharacter::freeflowEnd(UAnimMontage* animMontage, bool bInterrupted)
{
	Log("Freeflow end");
	bCanGoToNextFreeflow = true;
	ResetCollision();
}

void ADemonCharacter::StartFreeflow(bool enableDebug)
{
	Log("Start ragdoll?");
	GetMesh()->SetAllBodiesBelowSimulatePhysics("Hips", true);
	return;
	if (!bCanGoToNextFreeflow)
	{
		return;
	}
	auto inputDirection = GetInputDirection();


	TArray<AActor*> outActors = GetActorsFromSphere(AEnemy::StaticClass());
	Log("Num of actors in sphere: " + FString::FromInt(outActors.Num()));

	FVector bestVect = FVector::ZeroVector;
	FVector loc;

	if (inputDirection.Size() == 0.0)
	{
		inputDirection = GetActorForwardVector();
	}
	for (AActor* actor : outActors)
	{
		if (AEnemy* enemy = Cast<AEnemy>(actor))
		{
			ACharacter* freeflowChar = Cast<ACharacter>(enemy);
			Log("Found tag");
			auto vect = actor->GetActorLocation() - GetActorLocation();
			vect.Normalize();
			auto start = GetActorLocation();
			auto end = start + vect * 150;
			auto lineThickness = 100;
			if (enableDebug)
			{
				UKismetSystemLibrary::DrawDebugArrow(this, start, end, lineThickness, FLinearColor::Red, 5.0, 2.0);
			}

			start = actor->GetActorLocation();
			vect *= -1;

			end = start + vect * (freeflowChar->GetCapsuleComponent()->GetScaledCapsuleRadius() + freeflowDisFromTarget);
			vect *= -1;

			if (FVector::DotProduct(bestVect, inputDirection) < FVector::DotProduct(vect, inputDirection) &&
				FVector::DotProduct(vect, inputDirection) > 0.5)
			{

				bestVect = vect;
				loc = end;
			}

			if (enableDebug)
			{
				UKismetSystemLibrary::DrawDebugArrow(this, start, end, lineThickness, FLinearColor::Green, 5.0, 2.0);
			}

		}
	}

	if (bestVect != FVector::ZeroVector)
	{
		Log("Found Best Vect");
		auto start = GetActorLocation();
		auto end = start + bestVect * 150;
		auto lineThickness = 100;
		if (enableDebug)
		{
			UKismetSystemLibrary::DrawDebugArrow(this, start, end, lineThickness, FLinearColor::Blue, 5.0, 2.0);
		}

	}
	else
	{
		return;
	}


	TArray<UAnimMontage*> FreeflowAttackMontageArray;
	FreeflowAttackMontageMap.GenerateKeyArray(FreeflowAttackMontageArray);

	if (FreeflowAttackMontageArray.Num() > 0)
	{
		auto randInt = FMath::Rand() % FreeflowAttackMontageArray.Num();
		if (FreeflowAttackMontageArray.IsValidIndex(randInt))
		{
			//Setting up character movement
			//GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
			GetMovementComponent()->StopMovementImmediately();
			//Modifying camera collision
			GetCameraBoom()->bDoCollisionTest = false;

			//Getting Anim notify time
			auto mont = FreeflowAttackMontageArray[randInt];
			auto moveToTime = getMontageAnimNotifyTime(mont, "FreeflowEndNotify");

			//Setting the time to move to object
			auto desiredTime = 0.5;

			auto rate = moveToTime / desiredTime;
			PlayMontage(mont, "Default");
			MainCharacterAnimInstance->Montage_SetPlayRate(mont, rate);
			FOnMontageEnded BlendOutDelegate;
			BlendOutDelegate.BindUObject(this, &ADemonCharacter::freeflowEnd);
			GetMesh()->GetAnimInstance()->Montage_SetBlendingOutDelegate(BlendOutDelegate, mont);



			auto newRot = FRotator::ZeroRotator;
			newRot.Yaw = bestVect.Rotation().Yaw;
			SetActorRotation(newRot);
			bCanGoToNextFreeflow = false;

			auto T = desiredTime;
			if (enableDebug)
			{
				UKismetSystemLibrary::DrawDebugSphere(this, loc, 30, 12, FLinearColor::Blue, 2.0);
			}
			MoveCharacterToRotationAndLocationIninterval(loc, GetActorRotation(), T);
		}
	}
}
void ADemonCharacter::canGoToNextfreeflow()
{
	Log("Character can proceed to new free flow");
	bCanGoToNextFreeflow = true;
	GetMovementComponent()->StopMovementImmediately();
	if (GetMesh()->GetAnimInstance()->GetCurrentActiveMontage())
	{
		auto currentMont = GetMesh()->GetAnimInstance()->GetCurrentActiveMontage();
		GetMesh()->GetAnimInstance()->Montage_SetPlayRate(currentMont);
	}

}
void ADemonCharacter::UpdatePlayerAttack(float DeltaTime)
{
	if (!MainCharacterAnimInstance->Montage_IsActive(AttackRushMontage))
	{
		return;
	}
	if (playerEnemy != nullptr)
	{
		float distanceFromEnemy = getDistanceFromCharacter(playerEnemy);
		if (attackRushMinimumDistance >= distanceFromEnemy)
		{
			MainCharacterAnimInstance->Montage_Stop(0.2, AttackRushMontage);
			PlayerAttack();
		}
	}
}
float ADemonCharacter::getDistanceFromCharacter(ACharacter* character)
{
	if (character)
	{
		float capsuleRadius = character->GetCapsuleComponent()->GetScaledCapsuleRadius();
		float totalDistance = FVector::Dist(GetActorLocation(), character->GetActorLocation());

		return  totalDistance - capsuleRadius;
	}
	Log("Not valid character");
	return -1.0;
}
void ADemonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Jumping
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ADemonCharacter::onMoveStarted);
		EnhancedInputComponent->BindAction(LeftMouseAction, ETriggerEvent::Started, this, &ADemonCharacter::LeftMouseClick);
		EnhancedInputComponent->BindAction(LeftShiftAction, ETriggerEvent::Started, this, &ADemonCharacter::ShiftClick);
		EnhancedInputComponent->BindAction(LeftShiftAction, ETriggerEvent::Completed, this, &ADemonCharacter::ShiftClickEnd);
		EnhancedInputComponent->BindAction(LeftCtrltAction, ETriggerEvent::Started, this, &ADemonCharacter::LeftCTRLClick);
		EnhancedInputComponent->BindAction(LeftCtrltAction, ETriggerEvent::Completed, this, &ADemonCharacter::LeftCTRLClickEnd);
		EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Started, this, &ADemonCharacter::RightMouseClick);
		EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Completed, this, &ADemonCharacter::RightMouseClickEnd);

		EnhancedInputComponent->BindAction(EPressAction, ETriggerEvent::Completed, this, &ADemonCharacter::EKeyActionPress);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADemonCharacter::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADemonCharacter::StopJumping);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

}
void ADemonCharacter::StartJump()
{
	bJumpButtonIsPressed = true;
	if (GetIsDodgeAnimationPlaying() || GetIsAttackAnimationPlaying())
	{
		if (LaunchAttackMontage)
		{
			UAnimInstance* AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;
			AnimInstance->Montage_Stop(0.0, GetCurrentMontage());
			SetMovementState(EDemonMovementState::MS_Normal);
			PlayMontage(LaunchAttackMontage, "Default", 1.0);
			GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
			BindMontage(LaunchAttackMontage, [this](UAnimMontage* mont, bool interrupted) {
				GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Falling;
			});
			return;
		}
	}
	if (InAir())
	{
		return;
	}
	if (GetCurrentMontage())
	{
		if (GetCurrentMontage() == JumpLandMontage)
		{
			MainCharacterAnimInstance->Montage_Stop(0.0, JumpLandMontage);
		}
	}

	if (JumpStartMontage)
	{
		
		if (0)
		{
			PlayMontage(JumpStartMontage, "JumpRun");
		}
		else
		{
			PlayMontage(JumpStartMontage);
		}

		float time = getMontageAnimNotifyTime(JumpStartMontage, "JumpLaunch");
		if (time < 0.0)
		{
			Super::Jump();
		}
	}
	else
	{
		Super::Jump();
	}
}
void ADemonCharacter::StopJumping()
{
	bJumpButtonIsPressed = false;
	ToggleBoost(false, false);
	Super::StopJumping();
}
void ADemonCharacter::LeftCTRLClick()
{
	Log("left ctrl click");
	bLeftCtrlButtonIsHeld = true;

	if (MainCharacterAnimInstance->Montage_IsActive(GrabMontage))
	{
		StopAnimMontage(GrabMontage);
		auto arr = GetActorsFromSphere(AEnemy::StaticClass(), 250, true);
		bool enumyFound = false;
		for (AActor* actor : arr)
		{
			if (AEnemy* enem = Cast<AEnemy>(actor))
			{
				Log("Found enemy");
				if (MainCharacterAnimInstance->Montage_IsActive(GrabMontage))
				{
					StopAnimMontage(GrabMontage);
				}

				enem->setEnableRagdoll(false);
				enem->DetachRootComponentFromParent();
				enem->GetCapsuleComponent()->SetCollisionProfileName("Pawn");
				enem->GetMesh()->SetCollisionProfileName("CharacterMesh");
				enem->SetActorRotation(GetActorRotation());
				enem->GetCapsuleComponent()->AddRelativeRotation(FRotator(0, 180, 0));
				enem->GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Falling;
				enem->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
				enem->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
				enumyFound = true;
			}
		}
		return;
	}


	auto arr = GetActorsFromSphere(AEnemy::StaticClass(), 250, true);
	bool enumyFound = false;
	for (AActor* actor : arr)
	{
		if (AEnemy* enem = Cast<AEnemy>(actor))
		{
			Log("Found enemy");
			if (GrabMontage)
			{
				PlayAnimMontage(GrabMontage);
			}
			enem->setEnableRagdoll(true);
			FName SocketName = "handLeftPalmForward";
			FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
			enem->SetActorLocation(SocketLocation);
			FRotator SocketRotation = GetMesh()->GetSocketRotation(SocketName);
			enem->SetActorRotation(SocketRotation);

			enem->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			enem->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			enem->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			enem->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			enem->GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			enem->GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
			enumyFound = true;
			break;
		}
	}

	if (!enumyFound)
	{
		Log("No enemy to grab! :(");
	}
}
void ADemonCharacter::LeftCTRLClickEnd()
{
	Log("left ctrl end");
	bLeftCtrlButtonIsHeld = false;
}
void ADemonCharacter::LeftMouseClick()
{
	Log("left mouse click");
	if (bLeftCtrlButtonIsHeld)
	{
		StartFreeflow();
	}
	else
	{
		PlayerAttack();
	}
}
void ADemonCharacter::ShiftClick()
{
	Log("Shift click");
	PlayerDodge();
	SetMovementState(EDemonMovementState::MS_Sprint);
}
void ADemonCharacter::ShiftClickEnd()
{
	Log("Shift click");
}
void ADemonCharacter::RightMouseClick()
{
	Log("Right mouse click");
	bRightMouseButtonIsPressed = true;

	if (BlockParryChainMap.Num() > 0)
	{
		TArray<UAnimMontage*> montages;
		BlockParryChainMap.GenerateKeyArray(montages);

		UAnimMontage* montage = montages[0];

		bPlayerCanParry = true;

		PlayMontage(montage);
	}
}
void ADemonCharacter::RightMouseClickEnd()
{
	bRightMouseButtonIsPressed = false;

	UAnimMontage* mont = GetCurrentMontage();

	TArray<UAnimMontage*> keyMontages;
	TArray<UAnimMontage*> valueMontages;

	BlockParryChainMap.GenerateKeyArray(keyMontages);
	BlockParryChainMap.GenerateValueArray(valueMontages);

	if (keyMontages.Contains(mont) || valueMontages.Contains(mont))
	{
		StopAnimMontage(mont);
		bPlayerCanParry = false;
	}
}

void ADemonCharacter::ToggleBoost(bool reset, bool activate)
{
	if (reset)
	{
		BoostTime = BoostMaxTime;
	}
	bIsGliding = activate;
	float newAirontrol = defaultAirControl;
	if (bIsGliding)
	{
		newAirontrol = 5.0;
	}
	GetCharacterMovement()->AirControl = newAirontrol;
}
void ADemonCharacter::UpdateBoost(float DeltaTime)
{
	if (!bIsGliding)
	{
		return;
	}
	BoostTime -= DeltaTime;
	LaunchCharacter(FVector(0, 0, 120), false, true);
	if (BoostTime <= 0.0)
	{
		ToggleBoost(false, false);
	}
}
void ADemonCharacter::Swing(float DeltaTime, bool enableDebug)
{
	if (!bEnableSwing)
	{
		return;
	}
	FVector Vn = OrbitPoint - GetActorLocation();
	Vn.Normalize();
	FVector Vt = FVector::CrossProduct(GetActorRightVector(), Vn);
	Vt.Normalize();

	FVector PlayerToLocVect = GetActorLocation() - OrbitPoint;
	PlayerToLocVect.Normalize();

	FVector projectedVector = FVector::ZeroVector;

	Vt.Normalize();



	auto DirVect = OrbitPoint - GetActorLocation();
	auto DistPlayToOrbit = FVector::Dist(OrbitPoint, GetActorLocation());
	DirVect.Normalize();
	auto xyVelocity = Vt;


	auto orbitRadius = 200;
	auto speed = SwingSpeed;
	auto Chord = speed * DeltaTime;

	auto SinHalfAng = Chord / (2.0 * orbitRadius);
	auto CosHalfAngle = FMath::Sqrt(1 - SinHalfAng * SinHalfAng);
	auto upVectorStrength = DirVect * SinHalfAng * Chord;
	auto ForwardVelocity = xyVelocity * CosHalfAngle * Chord;
	auto playerVelocityDir = ForwardVelocity + upVectorStrength;

	playerVelocityDir.Normalize();

	GetCharacterMovement()->Velocity = playerVelocityDir * speed;

	if (enableDebug)
	{
		auto UpVect = Vn;
		auto RightVect = GetActorRightVector();

		DrawDebugSphere(GetWorld(), OrbitPoint, 50, 32, FColor::Blue);
		UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + UpVect * 200, 50, FLinearColor::White, 0.0, 15);
		UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + RightVect * 200, 50, FLinearColor::White, 0.0, 15);
		UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + playerVelocityDir * 250, 50, FLinearColor::Gray, 0.0, 15);
	}
}
void ADemonCharacter::StartSwing()
{
	if (InAir())
	{
		EnableSwing();
		return;
	}
	Log("Starting swing");
	float testHeight = 1000;
	//Player Gravity
	float G = -1 * GetMovementComponent()->GetGravityZ();
	float initialVelocity = FMath::Sqrt(2 * G * testHeight);
	float T = initialVelocity / G;

	LaunchCharacter(initialVelocity * GetActorUpVector(), 0, 0);
	Delay(T, "EnableSwing");
}
void ADemonCharacter::EnableSwing()
{
	OrbitPoint = GetActorForwardVector() * 300 + GetActorUpVector() * 300 + GetActorLocation();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	bEnableSwing = true;
}
void ADemonCharacter::PlayerDodgeEnd(UAnimMontage* animMontage, bool bInterrupted)
{
	Log("Player dodg ended");
	playerCanDodge = true;
	if (bInterrupted)
	{
		//Do nothing
	}
}
void ADemonCharacter::NextDodge()
{
	if (DodgeMontageArray.Num() > 0)
	{
		Log("Playing next dodge?");
		//PlayMontage(DodgeMontageArray[0]);

	}

	playerCanDodge = true;
}
void ADemonCharacter::PlayerAttack()
{
	if (getHitReactionMontageIsActive())
	{
		Log("Player Hit");
		return;
	}
	if (!playerCanDodge)
	{
		Log("Player is dodging");
		return;
	}
	if (!playerCanAttck)
	{
		Log("Player cannot attack");
		bAttackCacheEnabled = true;
		return;
	}
	if (MainCharacterAnimInstance->Montage_IsActive(GrabMontage))
	{
		if (ThrowMontage)
		{
			PlayMontage(ThrowMontage);
			return;
		}
	}
	TArray<UAnimMontage*> AttackArray = AttackMontageArray;
	int AttackIndex = 0;
	;	if (AttackArray.Num() == 0)
	{
		Log("Empty Attack array");
		return;
	}
	if (AttackArray.IsValidIndex(currentAttackIndex))
	{

		AttackIndex = currentAttackIndex;
	}
	else
	{
		currentAttackIndex = 0;
	}

	auto attackMontage = AttackArray[AttackIndex];

	if (playerEnemy)
	{
		auto rot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), playerEnemy->GetActorLocation());
		auto newRot = GetActorRotation();
		newRot.Yaw = rot.Yaw;
		float moveToTime = attackMontage->GetDefaultBlendInTime();

		//MoveCharacterToRotationAndLocationIninterval(GetActorLocation(), newRot, moveToTime);
		SetActorRotation(newRot);
		float distanceFromEnemy = getDistanceFromCharacter(playerEnemy);
		if (distanceFromEnemy > attackRushMinimumDistance && bEnableAttackRush)
		{
			PlayMontage(AttackRushMontage);
			return;
		}
	}
	GetCharacterMovement()->StopMovementImmediately();
	TArray<FString> AttackTags;
	if (AttackMontageMap.Contains(attackMontage))
	{
		FString str = *AttackMontageMap.Find(attackMontage);
		str.ParseIntoArray(AttackTags, TEXT(","), true);
	}
	else
	{
		Log("Montage has no tags");
	}
	//UGameplayStatics::PlaySound2D(this, AttackGrunt);
	const FVector headLoc =  GetMesh()->GetSocketLocation("Head");
	UGameplayStatics::PlaySoundAtLocation(this, AttackGrunt, headLoc);
	PlayMontage(attackMontage);
	if (AttackTags.Contains("MirrorLowerArm"))
	{
		Log("Mirroring lower arm");
		LowerArmState = ELowerArmState::LAS_Mirror;
	}
	else if (AttackTags.Contains("DisableLowerArm"))
	{
		LowerArmState = ELowerArmState::LAS_Disabled;
	}
	else
	{
		LowerArmState = ELowerArmState::LAS_Normal;
	}

	if (AttackTags.Contains("Mirror"))
	{
		Log("Miroring charaacter");
		MainCharacterAnimInstance->setEnableMirror(true);

	}

	BindMontage(attackMontage, "PlayerAttackEnd");

	playerCanAttck = false;
	currentAttackIndex++;

}
void ADemonCharacter::PlayerAttackEnd(UAnimMontage* animMontage, bool bInterrupted)
{
	if (bInterrupted)
	{


	}
	else
	{
		currentAttackIndex = 0;
	}
	LowerArmState = ELowerArmState::LAS_Normal;
	MainCharacterAnimInstance->setEnableMirror(false);
	playerCanAttck = true;
	bAttackCacheEnabled = false;
}
void ADemonCharacter::NextAttack()
{
	playerCanAttck = true;
	if (bAttackCacheEnabled)
	{
		PlayerAttack();
		bAttackCacheEnabled = false;
	}
}
void ADemonCharacter::onMoveStarted(const FInputActionValue& Value)
{
	Log("Move Started");

	FVector2D MovementVector = Value.Get<FVector2D>();
	MovingForwardValue = MovementVector.Y;
	MovingRightValue = MovementVector.X;

	if (!InAir())
	{
		bStartCharcterMovementRotation = true;

		characterInitialYawRotation = GetActorRotation().Yaw;
		characterFinalYawRotation = GetInputDirection().Rotation().Yaw;

		FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetInputDirection().Rotation(), GetActorRotation());
		if (GetMovementState() == EDemonMovementState::MS_Sprint || GetMovementState() == EDemonMovementState::MS_WallRun)
		{
			return;
		}

		currentCharacterRotationTime = 0.0;
		float ratio = FMath::Abs(DeltaRotator.Yaw);
		targetCurrentRotationTime = 0.2 * ratio / 180.0;

		Log("Delta input rotation: " + FString::SanitizeFloat(ratio));

		ETurnState turnState = TurnInPlace(GetInputDirection().Rotation());

		if (turnState != ETurnState::ETS_None)
		{
			float sectionLength = targetCurrentRotationTime;
			UAnimMontage* turnMontage = nullptr;
			switch (turnState)
			{
			case ETurnState::ETS_Left:
				turnMontage = TurnBackLeftMontage;
				break;
			case ETurnState::ETS_Right:
				turnMontage = TurnBackRightMontage;
				break;
			case ETurnState::ETS_RightHalf:
				turnMontage = TurnRightMontage;
				break;
			case ETurnState::ETS_LeftHalf:
				turnMontage = TurnLeftMontage;
				break;
			default:
				break;
			}
			if (turnMontage)
			{
				Log("Turn montage found?");
				PlayMontage(turnMontage);
			}
		}
	}
}
void ADemonCharacter::onMoveEnd(const FInputActionValue& Value)
{
	Super::onMoveEnd(Value);
	//bStartCharcterMovementRotation = false;
	FVector2D MovementVector = Value.Get<FVector2D>();
	MovingForwardValue = MovementVector.Y;
	MovingRightValue = MovementVector.X;
	springArmSpeed = idleSpringArmSpeed;
	bStartCharcterMovementRotation = false;
	if (getIsRotationMontPlaying())
	{
		MainCharacterAnimInstance->StopAllMontages(0.2);
	}
	if (GetMovementState() == EDemonMovementState::MS_Sprint)
	{
		SetMovementState(EDemonMovementState::MS_Normal);
	}


	//Delay(idleSpringArmDelay, "EnableZoomToChar");
}
bool ADemonCharacter::getIsRotationMontPlaying() const
{
	return MainCharacterAnimInstance->Montage_IsActive(TurnLeftMontage) || MainCharacterAnimInstance->Montage_IsActive(TurnRightMontage) ||
		MainCharacterAnimInstance->Montage_IsActive(TurnBackRightMontage) || MainCharacterAnimInstance->Montage_IsActive(TurnBackLeftMontage);
}
void ADemonCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	MovingForwardValue = MovementVector.Y;
	MovingRightValue = MovementVector.X;

	if (GetMovementState() == EDemonMovementState::MS_Sprint || DodgeMontageArray.Contains(GetCurrentMontage()) || GetMovementState() == EDemonMovementState::MS_WallRun)
	{
		if (getCanCancelAnimMontage())
		{
			if (GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
			{
				auto currMont = GetMesh()->GetAnimInstance()->GetCurrentActiveMontage();
				if (currMont)
				{
					GetMesh()->GetAnimInstance()->Montage_Stop(currMont->GetDefaultBlendOutTime(), currMont);
					setCanCancelAnimMontage(false);
				}
			}
			return;
		}
	}
	if (bStartCharcterMovementRotation)
	{
		const float DeltaTime = GetWorld()->GetDeltaSeconds();
		if (getIsRotationMontPlaying())
		{
			float ratio = 0.0;
			float SpeedRatio = 0.0;
			if (!TurnCurveName.IsNone() && MainCharacterAnimInstance->GetCurveValue(TurnCurveName, ratio))
			{
				ratio = FMath::Abs(ratio);
				//Log("Ratio is: " + FString::SanitizeFloat(ratio));
				float newYaw = UKismetMathLibrary::RLerp(FRotator(0, characterInitialYawRotation, 0), FRotator(0, characterFinalYawRotation, 0), ratio, true).Yaw;
				SetActorRotation(FRotator(0, newYaw, 0));
				AddMovementInput(GetInputDirection(), ratio * MovementVector.Length());


				if (ratio >= 0.99)
				{
					SetActorRotation(FRotator(0, characterFinalYawRotation, 0));
					bStartCharcterMovementRotation = false;
					if (getIsRotationMontPlaying())
					{
						MainCharacterAnimInstance->StopAllMontages(0.2);
					}
				}
			}


			currentCharacterRotationTime += DeltaTime;
		}
		else
		{
			float ratio = currentCharacterRotationTime / targetCurrentRotationTime;
			float newYaw = UKismetMathLibrary::RLerp(FRotator(0, characterInitialYawRotation, 0), FRotator(0, characterFinalYawRotation, 0), ratio, true).Yaw;

			if (ratio >= 0.99)
			{
				SetActorRotation(FRotator(0, characterFinalYawRotation, 0));
				bStartCharcterMovementRotation = false;
			}
			currentCharacterRotationTime += DeltaTime;
		}
		return;
	}

	if (MovementVector.Length() < 0.5)
	{
		if (MovementVector.Length() > 0.25)
		{
			SetActorRotation(GetInputDirection().Rotation());
		}
		return;
	}
	if (!DynamicCameraComponent->isFocused())
	{
		UpdateCamera(GetWorld()->DeltaTimeSeconds);
		idleSpringArmSpeed = movementSpringArmSpeed;
		bStartCharcterMovementRotation = false;
		springArmSideLength = 0.0;
		springArmForwardLength = 0.0;
		springArmUpLength = 0.0;
	}

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FRotator XYRotation = FRotator(0, Rotation.Yaw, Rotation.Roll);

		// get forward vector
		const FVector ForwardDirection = UKismetMathLibrary::GetForwardVector(YawRotation);

		// get right vector 
		const FVector RightDirection = UKismetMathLibrary::GetRightVector(XYRotation);

		bool useLeanMove = true;

		// add movement 
		if (useLeanMove)
		{
			float RInterpSpeed = MovementRotationSpeed;

			FVector TotalDirection = GetInputDirection();

			const FRotator DesiredRotation = UKismetMathLibrary::RInterpTo(GetActorRotation(), TotalDirection.Rotation(), GetWorld()->DeltaTimeSeconds, RInterpSpeed);
			if (!MainCharacterAnimInstance->IsAnyMontagePlaying())
			{
				SetActorRotation(DesiredRotation);
			}

			//UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + TotalDirection * 100,50,FLinearColor::Blue);
			AddMovementInput(DesiredRotation.Vector(), MovementVector.Length());
		}
		else
		{
			AddMovementInput(RightDirection, MovementVector.X);
			AddMovementInput(ForwardDirection, MovementVector.Y);
		}

	}

	if (getCanCancelAnimMontage())
	{
		if (GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
		{
			auto currMont = GetMesh()->GetAnimInstance()->GetCurrentActiveMontage();
			if (currMont)
			{
				GetMesh()->GetAnimInstance()->Montage_Stop(currMont->GetDefaultBlendOutTime(), currMont);
				setCanCancelAnimMontage(false);
			}
		}
	}
}
void ADemonCharacter::SoftLock(float DeltaTime)
{
	int radius = 1200;
	TArray<AActor*> actors = GetActorsFromSphere(AEnemy::StaticClass(), radius);
	/*
	while (radius < 1200 && actors.Num() < 2)
	{
		actors = GetActorsFromSphere(AEnemy::StaticClass(),radius);
		radius += 200;
	}
	*/
	if (actors.Num() == 0)
	{
		return;
	}
	float bestAngle = -1.0;
	AEnemy* selectedEnemy = nullptr;
	FVector vectToComp = GetActorForwardVector();
	float bestDistance = 0.0;
	if (GetInputDirection().Size() > 0.0)
	{
		vectToComp = GetInputDirection();
	}
	for (AActor* actor : actors)
	{
		if (AEnemy* enemy = Cast<AEnemy>(actor))
		{
			if (!selectedEnemy)
			{
				selectedEnemy = enemy;
				continue;
			}

			float distance = FVector::Dist(GetActorLocation(), enemy->GetActorLocation());
			if (distance < bestDistance)
			{
				bestDistance = distance;
				selectedEnemy = enemy;
			}
			continue;
			FVector enemyToCharVect = enemy->GetActorLocation() - GetActorLocation();
			enemyToCharVect.Normalize();


			float dotProduct = FVector::DotProduct(enemyToCharVect, vectToComp);
			//Log("The dot product is: " + FString::SanitizeFloat(dotProduct));
			if (dotProduct > bestAngle)
			{
				bestAngle = dotProduct;
				selectedEnemy = enemy;
			}
		}
	}
	if (!selectedEnemy && playerEnemy)
	{
		playerEnemy->enableOutline(false);
		return;
	}
	else if (!selectedEnemy)
	{
		//Do nothing
	}
	else if (!playerEnemy)
	{
		selectedEnemy->enableOutline(true);
		playerEnemy = selectedEnemy;
	}
	else if (selectedEnemy != playerEnemy)
	{
		selectedEnemy->enableOutline(true);
		playerEnemy->enableOutline(false);
		playerEnemy = selectedEnemy;
	}
}
void ADemonCharacter::PlayerDodge()
{
	Log("Player Dodge commence");
	if (getHitReactionMontageIsActive())
	{
		return;
	}
	if (MovementState == EDemonMovementState::MS_Glide)
	{
		return;
	}
	if (!playerCanDodge)
	{
		return;
	}
	if (GetInputDirection().Size() > 0.0 && 0)
	{
		CancelAllDelay();
		MovementState = EDemonMovementState::MS_Glide;
		Log("Starting Dodge Glide");
		float T = PlayMontage(DodgeGlideStartMontage);
		if (T > 0.0)
		{
			T += DodgeGlideLength;
			Delay(T, "EndDodgeGlide");
		}
		return;
	}
	if (DodgeMontageArray.Num() == 0)
	{
		Log("No dodge attacks");
		return;
	}
	int dodgeIndex = 0;
	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetInputDirection().Rotation(), GetActorRotation());
	float rootMotionScale = 1.0;
	if (GetInputDirection().Size() == 0.0)
	{
		rootMotionScale = 2.0;
		dodgeIndex = 1;
	}
	if (!DodgeMontageArray.IsValidIndex(dodgeIndex))
	{
		Log("Not valid random index?");
		return;
	}
	auto DodgeMontage = DodgeMontageArray[dodgeIndex];
	PlayMontage(DodgeMontage, "Default", 1.0);
	playerCanDodge = false;
	FOnMontageEnded BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ADemonCharacter::PlayerDodgeEnd);
	GetMesh()->GetAnimInstance()->Montage_SetBlendingOutDelegate(BlendOutDelegate, DodgeMontage);
}
void ADemonCharacter::UpdateDodgeGlide(float DeltaTime)
{
	if (MovementState != EDemonMovementState::MS_Glide)
	{
		return;
	}
	FVector Direction = GetActorForwardVector();
	float speed = DodgeGlideSpeed;

	GetCharacterMovement()->Velocity = speed * Direction;
}
void ADemonCharacter::EndDodgeGlide()
{
	MovementState = EDemonMovementState::MS_Normal;
	Log("Endinging Dodge Glide");
}
bool ADemonCharacter::getJumpButtonisPressed()
{
	return bJumpButtonIsPressed;
}
void ADemonCharacter::setJumpButtonisPressed(bool isPressed)
{
	bJumpButtonIsPressed = isPressed;
}
bool ADemonCharacter::getCharacterLanded()
{
	return bCharacterlanded;
}
void ADemonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	Log("Landed");
	GetController()->ResetIgnoreMoveInput();
	if (!bIsClimbing)
	{
		if (GetInputDirection().Size() > 0)
		{
			PlayMontage(JumpLandMontage, "LandRun", 1.25);
		}
		else
		{
			PlayMontage(JumpLandMontage);

		}
		setCanCancelAnimMontage();
	}
	bCharacterlanded = true;
	bCycleRunnigJumpMirror = !bCycleRunnigJumpMirror;
	ToggleBoost(true, false);
}
void ADemonCharacter::UpdateHitbox(float deltaTime, bool bEnableRightPunch, bool enableDebug)
{
	if (bEnableLimbHitBox)
	{
		AttackHitbox(limbAttackSocketName);
	}
}
void ADemonCharacter::StartHitbox(float deltaTime, bool bEnableRightPunch, bool enableDebug)
{
	try
	{
		
		if (!MainCharacterAnimInstance)
		{
			Log("Anim instance not available?");
			return;
		}
		TArray<FAnimNotifyEvent> fAnimNotifyEvents = MainCharacterAnimInstance->ActiveAnimNotifyState;

		FName SocketName;
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
				default:
					break;
				}

				isFound = true;
			}

			AttackHitbox(SocketName);

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
	//Should be one of the hand locations

}
void ADemonCharacter::AttackHitbox(FName SocketName, bool bEnableDebug)
{
	FVector AttackPoint = GetActorLocation();
	float zLoc = AttackPoint.Z;
	AttackPoint = GetMesh()->GetSocketLocation(SocketName);
	FVector AttackCenterPoint = AttackPoint;
	AttackCenterPoint.Z = GetActorLocation().Z;

	FVector AttackVector = AttackCenterPoint - GetActorLocation();

	float AttackVectorSize = GetCapsuleComponent()->GetScaledCapsuleRadius();
	float AttackCenterDist = AttackVectorSize + GetCapsuleComponent()->GetScaledCapsuleRadius() + 50;

	AttackVector.Normalize();
	auto StartPoint = GetActorLocation();
	StartPoint.Z = AttackPoint.Z;
	auto midPoint = AttackCenterDist * AttackVector + GetActorLocation();

	auto start = midPoint;
	start.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	auto end = midPoint;
	end.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	auto radius = 20;

	FHitResult hitResult;

	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	radius = 30.0;
	bool didHit = UKismetSystemLibrary::SphereTraceSingleForObjects(this, AttackPoint, AttackPoint,
		radius, traceObjectTypes, false, actorsToIgnore, bEnableDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitResult, true);
	if (didHit)
	{
		if (AEnemy* enemy = Cast<AEnemy>(hitResult.GetActor()))
		{
			if (!actorsHit.Contains(enemy))
			{
				actorsHit.Add(enemy);

				attackCombo++;
				Log("Attack combo: " + FString::FromInt(attackCombo));
				CancelAllDelay();
				Delay(2.0, [this]() {
					attackCombo = 0;
					Log("Attack combo: " + FString::FromInt(attackCombo));
				});

				enemy->HitReact(this);
				//SpawnParticle(HitImpact, hitResult.Location, FRotator::ZeroRotator);
				// DynamicCameraComponent->SetupFocus(enemy);
				//NextAttack();
				if (HitNiagraImpact && 1)
				{
					float impactScale = 0.5;
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitNiagraImpact, enemy->GetActorLocation(),
						(StartPoint + AttackCenterDist * AttackVector).Rotation(), FVector(hitImpactSize), true, true, ENCPoolMethod::AutoRelease, true);

				}
				UGameplayStatics::PlaySoundAtLocation(this, PunchSound, hitResult.Location);
				//DrawDebugSphere(GetWorld(), hitResult.Location, 25, 12, FColor::Blue, false, 2.0);
			}
		}
	}
}
bool ADemonCharacter::getHitReactionMontageIsActive() const
{
	return HitReactionMontage == GetCurrentMontage() || HitReactionMontageArray.Contains(GetCurrentMontage());
}
float ADemonCharacter::HitReact(AActor* sender)
{
	if (GetIsDodgeAnimationPlaying())
	{
		return 0.0;
	}
	if (!HitReactionMontage)
	{
		Log("HitRection montage no good");
	}
	float T = 0.0;
	auto vector = GetActorLocation() - sender->GetActorLocation();
	FName sectionName = "Default";
	vector.Normalize();
	vector *= -1;
	if (FVector::DotProduct(GetActorForwardVector(), vector) < FVector::DotProduct(-GetActorForwardVector(), vector))
	{
		for (FCompositeSection compSec : HitReactionMontage->CompositeSections)
		{
			if (compSec.SectionName.IsEqual("HitBack"))
			{
				sectionName = "HitBack";
			}
		}
	}
	AEnemy* attacker = Cast<AEnemy>(sender);
	if (attacker)
	{
		auto yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), attacker->GetActorLocation()).Yaw;

		auto newRot = GetActorRotation();
		newRot.Yaw = yaw;
		if (sectionName.IsEqual("HitBack"))
		{
			newRot.Yaw = attacker->GetActorRotation().Yaw;
		}
		//MoveCharacterToRotationAndLocationIninterval(GetActorLocation(), newRot, hitReactMontage->GetDefaultBlendInTime());
		SetActorRotation(newRot);

		UAnimMontage* mont = GetCurrentMontage();
		auto hitReactMontage = HitReactionMontage;


		if (BlockParryChainMap.Contains(mont))
		{
			MainCharacterAnimInstance->ActiveAnimNotifyState;

			bool isFound = false;
			for (auto animNotifyState : MainCharacterAnimInstance->ActiveAnimNotifyState)
			{
				if (animNotifyState.NotifyName.IsEqual("ParryWindowNotifyState") && bPlayerCanParry)
				{
					hitReactMontage = *BlockParryChainMap.Find(mont);
					Log("Parry");
					isFound = true;
					attacker->setCanHitReact(true);
					attacker->HitReact(this);
					//PlayAnimMontage(*BlockParryChainMap.Find(mont));
				}
				else
				{
					//bPlayerCanParry = false;
				}
			}

			if (BlockReactArray.Num() > 0 && !isFound)
			{
				const int lastIndex = BlockReactArray.Num() - 1;
				hitReactMontage = BlockReactArray[lastIndex];
			}
		}

		T = BindAndPlayMontage(hitReactMontage, "HitReactEnd");
	}


	return T;
}
void ADemonCharacter::HitReactEnd(UAnimMontage* animMontage, bool bInterrupted)
{
	if (bRightMouseButtonIsPressed)
	{
		RightMouseClick();
	}
}

void ADemonCharacter::AttackRushEnd(UAnimMontage* animMontage, bool bInterrupted)
{
}

void ADemonCharacter::EKeyActionPress()
{
	if (bIsClimbing)
	{
		bIsClimbing = false;
	}
	else
	{
		//Needs work
		//StartWallClimb();
	}
}
void ADemonCharacter::StartWallClimb()
{
	FHitResult hitResult;
	bIsClimbing = SphereTrace(GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 50, 25, ETraceTypeQuery::TraceTypeQuery1, 2, hitResult);
	if (!bIsClimbing)
	{
		return;
	}
	CLimbNormal = hitResult.Normal;
	GetCharacterMovement()->bCanWalkOffLedges = false;

}
void ADemonCharacter::UpdateWallclimb(float DeltaTime, bool bEnableDebug)
{
	/*
	FVector PlayerCurrentGravityDir = GetCharacterMovement()->GetGravityDirection();
	FVector TargetGravityDirection = bIsClimbing ? CLimbNormal * -1 : FVector(0, 0, -1);
	float targetSpeed = bIsClimbing ? 5.0 : 10.0;
	FVector newGravityDirection = UKismetMathLibrary::VInterpTo_Constant(PlayerCurrentGravityDir, TargetGravityDirection, DeltaTime, targetSpeed);
	GetCharacterMovement()->SetGravityDirection(newGravityDirection);
	*/
	if (getMoveCharacterToIsActive())
	{
		Log("Character is moving");
		return;
	}
	if (EDemonMovementState::MS_WallRun == MovementState)
	{
		UpdateWallRun(DeltaTime);
		return;
	}

	if (EDemonMovementState::MS_Sprint != MovementState)
	{
		return;
	}

	if (MainCharacterAnimInstance->Montage_IsActive(MantleMontage))
	{
		FVector RootMotionVector = MainCharacterAnimInstance->getRootMotionDataFromActiveMontage();
		MantleMontage->GetAnimCompositeSection(0).GetLinkedSequence();

		Log("Mantle Vector: " + RootMotionVector.ToString());
		return;
	}

	if (GetInputDirection().Size() == 0.0)
	{
		return;
	}

	TArray<AActor*> ActorsToIgnore;
	FVector StartPoint = GetActorLocation();
	FVector EndPoint = StartPoint + GetInputDirection() * 1000;
	FHitResult Result;
	bool didHit = UKismetSystemLibrary::LineTraceSingle(this, StartPoint, EndPoint, ETraceTypeQuery::TraceTypeQuery1,
		false, ActorsToIgnore, EDrawDebugTrace::None, Result, true);

	// Want actor rotation to be close to input dir to make it look natural
	if (didHit && FMath::Abs(GetActorRotation().Yaw - GetInputDirection().Rotation().Yaw) < 45.0)
	{
		FHitResult WallHit = Result;
		float playerDistFromHit = FVector::Dist(WallHit.Location, GetActorLocation());
		StartPoint = WallHit.Location + GetActorUpVector() * 500.0;
		EndPoint = WallHit.Location;
		FHitResult FloorHit;

		didHit = SphereTrace(StartPoint, EndPoint, 30, ETraceTypeQuery::TraceTypeQuery1, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, FloorHit);

		if (didHit)
		{

			if (FloorHit.Location.Z - GetActorLocation().Z > 200.0 && playerDistFromHit < 50.0)
			{
				FVector WallNormal = WallHit.Normal;
				FRotator ActorRoatation = GetActorRotation();
				ActorRoatation.Yaw = WallNormal.Rotation().Yaw;
				ActorRoatation.Yaw += 180;

				GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
				MovementState = EDemonMovementState::MS_WallRun;

				MoveCharacterToRotationAndLocationIninterval(WallHit.Location + WallNormal * (GetCapsuleComponent()->GetScaledCapsuleRadius() + 50.0),
					ActorRoatation, 0.2, "MovementRotationCompleted");
			}
			/*
			if (playerDistFromHit <= 200.0)
			{
				if (FloorHit.ImpactPoint.Z < GetActorLocation().Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * .25 &&
					FloorHit.ImpactPoint.Z > GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * .25)
				{
					Log("Can mantle");
				}
				else
				{
					return;
				}
				if (!MantleMontage)
				{
					return;
				}
				GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
				GetMovementComponent()->StopMovementImmediately();
				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				auto floatInTime = MantleMontage->GetDefaultBlendInTime();
				auto newLoc = WallHit.ImpactPoint;
				newLoc.Z += mantleZOffsset;

				auto newRot = WallHit.Normal.Rotation().Yaw + 180;
				auto playerRot = GetActorRotation();
				//playerRot.Yaw = newRot;

				GetCameraBoom()->bDoCollisionTest = false;

				FOnMontageEnded BlendOutDelegate;


				float delay = PlayMontage(MantleMontage);
				BlendOutDelegate.BindUObject(this, &ADemonCharacter::MantleEnd);
				GetMesh()->GetAnimInstance()->Montage_SetBlendingOutDelegate(BlendOutDelegate, MantleMontage);
				GetController()->SetIgnoreMoveInput(true);
				Log("Mantle length is :" + FString::SanitizeFloat(delay));
				MoveCharacterToRotationAndLocationIninterval(newLoc, playerRot, floatInTime);
			}

			if (playerDistFromHit <= 500.0 && playerDistFromHit > 200.0)
			{
				StartPoint = WallHit.Location;
				StartPoint.Z += 1000;
				EndPoint = WallHit.Location;

				didHit = SphereTrace(StartPoint, EndPoint, 30.0, ETraceTypeQuery::TraceTypeQuery1, 0, Result);

				if (didHit)
				{
					FHitResult FloorCornerHit = Result;
					float height = FloorCornerHit.Location.Z - GetActorLocation().Z;

					if (height >= 1000)
					{
						return;
					}

					if (FloorCornerHit.Location.Z > GetActorLocation().Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight())
					{
						Log("EveryBody Jump");

						FVector LaunchVelocity;
						FVector LandLocation = FloorCornerHit.Location;
						LandLocation += GetInputDirection() * (GetCapsuleComponent()->GetScaledCapsuleRadius());
						LandLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
						SphereTrace(LandLocation, LandLocation, 10.0, ETraceTypeQuery::TraceTypeQuery1, 1, Result);
						float GravityScale = GetCharacterMovement()->GravityScale;
						SuggestProjectileVelocityCustomArc(LaunchVelocity, GetActorLocation(), LandLocation, GravityScale, 0.5);
						GetController()->SetIgnoreMoveInput(true);
						LaunchCharacter(LaunchVelocity, true, true);
					}
				}
			}
			*/
		}


	}

}
void ADemonCharacter::MovementRotationCompleted()
{
	Log("Move to location and rotation completed");


}
void ADemonCharacter::UpdateMovementRotation(float DeltaTime)
{
	if (MovementState == EDemonMovementState::MS_WallRun)
	{
		return;
	}
	if (MovementState == EDemonMovementState::MS_Normal)
	{
		GetCharacterMovement()->MaxWalkSpeed = orignalWalkSpeed;
	}
	int forwardDodgeIndex = 0;
	if (MovementState == EDemonMovementState::MS_Sprint || (DodgeMontageArray.IsValidIndex(forwardDodgeIndex) && DodgeMontageArray[forwardDodgeIndex] == GetCurrentMontage()))
	{
		float MovementSpeed = orignalWalkSpeed * SprintMultiplier;

		if (DodgeMontageArray.IsValidIndex(forwardDodgeIndex) && DodgeMontageArray[forwardDodgeIndex] == GetCurrentMontage())
		{
			MovementSpeed = orignalWalkSpeed * (SprintMultiplier + .25);
		}

		float DodgeAlpha = 0.0;
		if (MainCharacterAnimInstance->GetCurveValue("DodgeCurve", DodgeAlpha))
		{
			MovementSpeed *= DodgeAlpha;
			Log("Alpha value is: " + FString::SanitizeFloat(DodgeAlpha));
		}
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;


		UpdateCamera(DeltaTime);

		if (!DodgeMontageArray.Contains(GetCurrentMontage()))
		{
			float RInterpSpeed = 2.5;
			FVector TotalDirection = GetActorForwardVector();

			if (GetInputDirection().Size() != 0.0)
			{
				TotalDirection = GetInputDirection();
			}

			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + TotalDirection * 200, FColor::Blue);

			const FRotator DesiredRotation = UKismetMathLibrary::RInterpTo(GetActorRotation(), TotalDirection.Rotation(), GetWorld()->DeltaTimeSeconds, RInterpSpeed);

			SetActorRotation(DesiredRotation);
		}


		//UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + TotalDirection * 100,50,FLinearColor::Blue);
		//AddMovementInput(DesiredRotation.Vector());

	}
}
void ADemonCharacter::UpdateHeadLookAtLocation(float DeltaTime)
{
	HeadLookatLocation = GetMesh()->GetSocketLocation("head") + FollowCamera->GetForwardVector() * 500;
	FHitResult result;
	SphereTrace(HeadLookatLocation, HeadLookatLocation, 25, ETraceTypeQuery::TraceTypeQuery1, 1, result);
}
void ADemonCharacter::UpdateCableActor(float DeltaTime)
{
	if (CableComponent)
	{

	}
}
void ADemonCharacter::UpdateWallRun(float DeltaTime)
{

	if (MovementState != EDemonMovementState::MS_WallRun)
	{
		return;
	}
	float wallRunSpeed = 600.0;
	if (MainCharacterAnimInstance->Montage_IsActive(WallRunJumpLedgeMontage)
		|| MainCharacterAnimInstance->Montage_IsActive(WallRunJumpEndMontage))
	{
		WallRunLedgeJumpCurrentTime += DeltaTime;
		float alpha = WallRunLedgeJumpCurrentTime / WallRunLedgeJumpTotalTime;
		Log("The alpha is: " + FString::SanitizeFloat(alpha));
		if (alpha < 1.0)
		{
			FVector nextLoc = UBezierCurve::GetPointAtRatio(p0, p1, p2, alpha);
			FVector displacement = nextLoc - prevPoint;
			FVector velocity = displacement / DeltaTime;

			auto rot = (GetActorUpVector()).Rotation();
			auto rightVector = UKismetMathLibrary::GetUpVector(rot);
			auto RightVect = rightVector * MovingRightValue * -1.0;

			velocity += RightVect * wallRunSpeed;
			GetCharacterMovement()->Velocity = velocity;
			prevPoint = nextLoc;
		}
		else
		{
			Log("Ending wall run montage");
			if (MainCharacterAnimInstance->Montage_IsActive(WallRunJumpEndMontage))
			{
				MainCharacterAnimInstance->Montage_Stop(0.2, WallRunJumpEndMontage);
				SetMovementState(EDemonMovementState::MS_Normal);
				GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
				return;
			}
			MainCharacterAnimInstance->Montage_Stop(0.2, WallRunJumpLedgeMontage);
		}
		return;
	}

	TArray<AActor*> ActorsToIgnore;

	FHitResult frontWalHiit;
	FVector wallUpVect = FVector::ZeroVector;
	FVector wallSurfaceNormal = FVector::ZeroVector;
	bool frontWallDidHit = UKismetSystemLibrary::LineTraceSingle(this, GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * (GetCapsuleComponent()->GetScaledCapsuleRadius() + 100),
		ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, frontWalHiit, true);

	if (!frontWallDidHit)
	{
		float T = 2.5;
		if (WallRunJumpEndMontage)
		{
			T = PlayMontage(WallRunJumpEndMontage);
			auto lambda = [this](UAnimMontage* animMontage, bool bInterrupted)
				{
					Log("Calling lambda");
					SetMovementState(EDemonMovementState::MS_Normal);
					GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
				};
			BindMontage(WallRunJumpEndMontage, lambda);
		}
		else
		{
			Log("Jump ledge end montage not available");
		}

		FVector midpoint = GetActorUpVector() * (250) +
			0 + GetActorLocation();

		FVector endpoint = midpoint + GetActorForwardVector() * 100;

		WallRunLedgeJumpCurrentTime = 0.0;
		WallRunLedgeJumpTotalTime = T;
		p0 = GetActorLocation();
		p1 = midpoint;
		p2 = endpoint;
		prevPoint = p0;
		return;
	}

	if (frontWalHiit.GetActor())
	{
		wallUpVect = frontWalHiit.GetActor()->GetActorUpVector();
	}

	wallSurfaceNormal = frontWalHiit.Normal;

	FVector StartPoint = GetActorLocation();
	FVector EndPoint = StartPoint + GetActorUpVector() * 300.0;
	FHitResult Result;
	bool didHit = UKismetSystemLibrary::LineTraceSingle(this, StartPoint, EndPoint, ETraceTypeQuery::TraceTypeQuery1,
		false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, Result, true);

	// Want actor rotation to be close to input dir to make it look natural
	if (didHit)
	{
		FHitResult WallHit = Result;
		float playerDistFromHit = FVector::Dist(WallHit.Location, GetActorLocation());

		playerDistFromHit += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		StartPoint = GetActorUpVector() * playerDistFromHit + GetActorLocation();
		EndPoint = StartPoint;

		StartPoint += GetActorForwardVector() * (-500);

		FHitResult FloorHit;

		didHit = SphereTrace(StartPoint, EndPoint, 30, ETraceTypeQuery::TraceTypeQuery1, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, FloorHit);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		if (didHit)
		{
			FVector playerMoveToLoc = FloorHit.Location + FloorHit.Normal * (GetCapsuleComponent()->GetScaledCapsuleRadius());
			float T = 2.5;
			if (WallRunJumpLedgeMontage)
			{
				T = PlayMontage(WallRunJumpLedgeMontage);
			}

			FVector midpoint = GetActorForwardVector() * (-FVector::Dist(playerMoveToLoc, EndPoint)) + GetActorLocation();

			WallRunLedgeJumpCurrentTime = 0.0;
			WallRunLedgeJumpTotalTime = T;
			p0 = GetActorLocation();
			p1 = midpoint;
			p2 = playerMoveToLoc;
			prevPoint = p0;
			return;
		}
	}

	FVector TotalDirection = GetActorUpVector();

	if (GetInputDirection().Size() != 0.0)
	{
		auto followCamera = GetFollowCamera();
		auto rot = (GetActorUpVector()).Rotation();
		auto rightVector = FVector::CrossProduct(wallSurfaceNormal, wallUpVect);
		auto RightVect = rightVector * MovingRightValue;
		auto InputDirection = RightVect;
		InputDirection.Normalize();

		TotalDirection = InputDirection;
	}

	FVector currentVel = GetCharacterMovement()->Velocity;
	currentVel.Normalize();

	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + TotalDirection * 200, FColor::Blue);
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + currentVel * 200, FColor::Red);

	GetCharacterMovement()->Velocity = UKismetMathLibrary::VInterpTo_Constant(currentVel, TotalDirection, DeltaTime, 3.5) * wallRunSpeed;

}
void ADemonCharacter::UpdateDodge(float DeltaTime)
{
}
bool ADemonCharacter::GetIsAttackAnimationPlaying()
{
	if (!GetCurrentMontage())
	{
		return false;
	}
	return AttackMontageMap.Contains(GetCurrentMontage()) || AttackMontageArray.Contains(GetCurrentMontage());
}
bool ADemonCharacter::GetIsFreeflowAnimationPlaying()
{
	if (!GetCurrentMontage())
	{
		return false;
	}
	return FreeflowAttackMontageMap.Contains(GetCurrentMontage());
}
bool ADemonCharacter::GetIsDodgeAnimationPlaying()
{
	if (!GetCurrentMontage())
	{
		return false;
	}
	return DodgeMontageArray.Contains(GetCurrentMontage());
}

void ADemonCharacter::SetMovementState(EDemonMovementState movemntState)
{
	MovementState = movemntState;
}