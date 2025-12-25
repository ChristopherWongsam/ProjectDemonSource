// Fill out your copyright notice in the Description page of Project Settings.


#include <Kismet/KismetSystemLibrary.h>
#include "BasicEnemy.h"

void ABasicEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void ABasicEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UpdateHeadFindPlayer(DeltaTime);
	//UpdateMoveToPlayer(DeltaTime);
}

void ABasicEnemy::UpdateHeadFindPlayer(float DeltaTime)
{
	if (bPlayerFound || !bEnemyCanAttack)
	{
		return;
	}
	if (!GetMesh()->DoesSocketExist(headForwardEndSocketName) || !GetMesh()->DoesSocketExist(headForwardStartSocket))
	{
		Log("Cannot look at player: Socket in skeletons does not exist");
	}
	FVector headOrigin = GetMesh()->GetSocketLocation(headForwardStartSocket);
	FVector headEndOrigin = GetMesh()->GetSocketLocation(headForwardEndSocketName);
	FVector EndPoint = headEndOrigin - headOrigin;
	EndPoint.Normalize();
	FHitResult hitResult;

	TArray<AActor*> actors = GetActorsFromSphere(ADemonCharacter::StaticClass(), 2500);
	if (actors.Num() > 0)
	{
		if (ADemonCharacter* enemy = Cast<ADemonCharacter>(actors[0]))
		{
			Log("Main Character found");
			bPlayerFound = true;
			EnemyController->MoveToActor(enemy, 20.0);
			setEnableCharacterToTargetRotation(true);
		}
	}
}
void ABasicEnemy::UpdateMoveToPlayer(float DeltaTime)
{
	if (!MyPlayerCharacter)
	{
		return;
	}
	if (!bPlayerFound)
	{
		return;
	}

	float playerToEnemyDistance = FVector::Dist(MyPlayerCharacter->GetActorLocation(), GetActorLocation());
	//float yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), MyPlayerCharacter->GetActorLocation()).Yaw;
	
	if (playerToEnemyDistance > EnemyAttackRange)
	{
		GetCharacterMovement()->MaxWalkSpeed = 500;
		EnemyController->MoveToActor(MyPlayerCharacter, 1.0);
		bEnablePlayerRangeDecsion = true;
		EnemyMoveToLocation = GetActorLocation();
		CancelAllDelay();
	}
	else
	{
		EnemyController->StopMovement();
	}

	if (playerToEnemyDistance <= EnemyAttackRange)
	{
		if (!bEnablePlayerRangeDecsion)
		{
			return;
		}

		try
		{
			if (HitReactMontageArray.Contains(GetCurrentMontage()))
			{
				return;
			}

			Log("Hello");

			if (!GetCurrentMontage())
			{
				UAnimMontage* dodgeMont = nullptr;
				auto chance = UKismetMathLibrary::RandomBool();
				if (chance)
				{
					LogScreen("Playing dodge?");
					dodgeMont = DodgeMontage;
					SetAnimRootMotionTranslationScale(0.75);
				}

				if (dodgeMont)
				{
					setEnableCharacterToTargetRotation(false);
					PlayMontage(dodgeMont);
					EnemyAnimInstance->bindMontageRootMotionModifier(dodgeMont, DodgeMovementScale);
					BindMontage(dodgeMont, "OnDodgeEnd");
				}
				else if (LightAttackMontage)
				{
					LogScreen("Enemy Attack?");
					Attack();
				}
				else
				{
					EnemyController->MoveToActor(MyPlayerCharacter, 1.0);
				}
			}
		}
		catch (const std::exception& e)
		{
			Log(e.what());
		}
		
	}
}
void ABasicEnemy::OnDodgeEnd(UAnimMontage* Montage, bool interrupted)
{
	LogScreen("Dodge end");
	int chance = UKismetMathLibrary::RandomInteger64InRange(0, 2);
	//setEnableCharacterToTargetRotation(false);
	SetAnimRootMotionTranslationScale(1.0);

	float playerToEnemyDistance = 0.0;

	if (MyPlayerCharacter)
	{
		playerToEnemyDistance = FVector::Dist(MyPlayerCharacter->GetActorLocation(), GetActorLocation());
	}

	if (playerToEnemyDistance <= EnemyAttackRange)
	{
		return;
	}

	UAnimMontage* nextMontage = nullptr;

	if (DodgeMontageChainMap.Contains(Montage))
	{
		FString montName;
		Montage->GetName(montName);
		nextMontage = *DodgeMontageChainMap.Find(Montage);
		nextMontage->GetName(montName);
	}

	if (nextMontage && MyPlayerCharacter)
	{
		FRotator newRot = GetActorRotation();
		newRot.Yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), MyPlayerCharacter->GetActorLocation()).Yaw;
		//SetActorRotation(newRot);
		SetAnimRootMotionTranslationScale(1.25);
		PlayMontage(nextMontage);
		BindMontage(nextMontage, "OnDodgeEnd");
	}
}

float ABasicEnemy::Attack()
{
	if (LightAttackMontage)
	{
		const float T = PlayMontage(LightAttackMontage);
		setCanHitReact(false);
		return T;
	}
	return 0.0;
}
void ABasicEnemy::EnablePlayerRangeDecision()
{
	LogScreen("Enable trigger EnablePlayerRangeDecision");
	bEnablePlayerRangeDecsion = true;
}
FVector ABasicEnemy::GetStrafeLocation()
{
	FVector ForwardLoc = GetActorForwardVector() * 50;
	int chance = UKismetMathLibrary::RandomInteger64InRange(0, 1);
	FVector RightLoc = GetActorRightVector() * 200;
	if (chance)
	{
		RightLoc *= -1;
	}

	FVector location = ForwardLoc + RightLoc + GetActorLocation();
	return location;
}
bool ABasicEnemy::GetIsAttackAnimationPlaying()
{
	return GetCurrentMontage() == LightAttackMontage;
}
bool ABasicEnemy::GetIsDodgeAnimationPlaying()
{
	return GetCurrentMontage() == DodgeMontage;
}
float ABasicEnemy::HitReact(AActor* sender)
{
	if (!getCanHitReact())
	{
		LogScreen("Can't hit enemy: enemy is Attacking");
		return 0.0;
	}
	if (GetIsDodgeAnimationPlaying() && false)
	{
		LogScreen("Can't hit enemy: enemy is dodging");
		return 0.0;
	}
	int index = 0;
	if (HitReactMontageArray.Num() == 0)
	{
		LogScreen("Enemy hit react montage array empty");
		return 0.0;
	}
	if (GetCurrentMontage())
	{
		StopAnimMontage(GetCurrentMontage());
	}
	int attackCombo = 0;
	ADemonCharacter* demonCharacter = Cast<ADemonCharacter>(sender);

	if (demonCharacter)
	{
		attackCombo = demonCharacter->getAttackCombo();
	}

	bool chance = UKismetMathLibrary::RandomBool();

	if (chance && bEnableHitReactDodge && attackCombo == 1)
	{
		PlayMontage(DodgeMontage);
		return 0.0;
	}

	auto vector = GetActorLocation() - sender->GetActorLocation();
	FName sectionName = "Default";
	vector.Normalize();
	vector *= -1;
	if (FVector::DotProduct(GetActorForwardVector(), vector) < FVector::DotProduct(-GetActorForwardVector(), vector))
	{
		//sectionName = "HitBack";
	}
	auto hitReactMontage = HitReactMontageArray[index];
	float T = PlayMontage(hitReactMontage, sectionName, 1.0);
	BindMontage(hitReactMontage, "HitReactEnd");
	auto yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), sender->GetActorLocation()).Yaw;

	auto newRot = GetActorRotation();
	newRot.Yaw = yaw;
	SetActorRotation(newRot);

	return T;
}

void ABasicEnemy::OnAttackEnd(UAnimMontage* Montage, bool interrupted)
{
	setEnableCharacterToTargetRotation(true);
}
