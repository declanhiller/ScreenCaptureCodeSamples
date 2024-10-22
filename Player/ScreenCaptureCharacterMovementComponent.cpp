// Fill out your copyright notice in the Description page of Project Settings.


#include "ScreenCaptureCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

UScreenCaptureCharacterMovementComponent::UScreenCaptureCharacterMovementComponent()
{
	//setup default values
	MaxWalkSpeed = 250;
	StoredWalkSpeed = MaxWalkSpeed;
	RunSpeed = 450;
	MaxWalkSpeedCrouched = 100;
	JumpZVelocity = 300;
	CrouchSpeed = 250;
	StunnedSpeed = 10;
}

void UScreenCaptureCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	StoredWalkSpeed = MaxWalkSpeed;
}


void UScreenCaptureCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UScreenCaptureCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	//figure out whether we should uncrouch or crouch based on state
	const bool bIsCrouching = IsCrouching();
	if (bIsCrouching && (!bWantsToCrouch || !CanCrouchInCurrentState()))
	{
		UnCrouch(false);
	}
	else if (!bIsCrouching && bWantsToCrouch && CanCrouchInCurrentState())
	{
		Crouch(false);
		bIsRunning = false;
	}else if(bWantsToRun && !bIsRunning)
	{
		MaxWalkSpeed = RunSpeed;
		bIsRunning = true;
	} else if(!bWantsToRun && bIsRunning)
	{
		MaxWalkSpeed = StoredWalkSpeed;
		bIsRunning = false;
	}

	if(bStunned)
	{
		MaxWalkSpeed = StunnedSpeed;
	}

	CrouchTick(DeltaSeconds);
}

void UScreenCaptureCharacterMovementComponent::Stun(float TimeToStun)
{
	bStunned = true;
	bWantsToRun = false;
	GetWorld()->GetTimerManager().SetTimer(StunTimerHandle, this, &UScreenCaptureCharacterMovementComponent::UnStun, TimeToStun);
}

void UScreenCaptureCharacterMovementComponent::UnStun()
{
	bStunned = false;

	//HELLA HACKY LMAOOOOOOOOOOOOOOOOOOOOOOOOOOO
	bWantsToRun = false;
	bIsRunning = true;
	
}

void UScreenCaptureCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	//make sure we can actually crouch
	if (!HasValidData())
	{
		return;
	}

	//confirm that player can crouch
	if (!bClientSimulation && !CanCrouchInCurrentState())
	{
		return;
	}
	
	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == GetCrouchedHalfHeight())
	{
		CharacterOwner->bIsCrouched = true;

		CharacterOwner->OnStartCrouch(0.f, 0.f);
		return;
	}

	//setup data to crouch
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedCrouchedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, GetCrouchedHalfHeight());
	TargetHalfHeight = ClampedCrouchedHalfHeight;
	StartHalfHeight = OldUnscaledHalfHeight;


	bIsRunningCrouchTickLogic = true;
	LerpAlphaValue = 0;

	CharacterOwner->bIsCrouched = true;
	
	bForceNextFloorCheck = true;
}

void UScreenCaptureCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	//make sure we have the valid data in character movement component
	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()
		->GetUnscaledCapsuleHalfHeight())
	{
		if (!bClientSimulation)
		{
			CharacterOwner->bIsCrouched = false;
		}
		CharacterOwner->OnEndCrouch(0.f, 0.f);
		return;
	}

	//get data for uncrouch
	const float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() -
		OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	//check above player's head to see if the player can actually uncrouch by sweeping
	
	const UWorld* MyWorld = GetWorld();
	const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	// Compensate for the difference between current capsule size and standing size
	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(
		SHRINK_HeightCustom,
		-SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	//calculate if uncrouching will encroach on geometry
	FVector StandingLocation = PawnLocation + FVector(
		0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
	bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel,
	                                                    StandingCapsuleShape, CapsuleParams, ResponseParam);

	//fully confirm if uncrouching will encroach on geometry
	if (bEncroached)
	{
		if (IsMovingOnGround())
		{
			// Something might be just barely overhead, try moving down closer to the floor to avoid it.
			const float MinFloorDist = UE_KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel,
				                                                    StandingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}

	//setup data to uncrouch
	if (!bEncroached)
	{
		StartHalfHeight = OldUnscaledHalfHeight;
		TargetHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		bIsRunningCrouchTickLogic = true;
		CharacterOwner->bIsCrouched = false;
		LerpAlphaValue = 0;
	}
	else
	{
		// bWantsToCrouch = true; //basically this line consumes crouch input
	}
}

bool UScreenCaptureCharacterMovementComponent::CanUnCrouch()
{
	if (!HasValidData())
	{
		return false;
	}

	//check data to see if player can uncrouch
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	const float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() -
		OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	const UWorld* MyWorld = GetWorld();
	const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	// Compensate for the difference between current capsule size and standing size
	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(
		SHRINK_HeightCustom,
		-SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	FVector StandingLocation = PawnLocation + FVector(
		0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
	bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel,
	                                                    StandingCapsuleShape, CapsuleParams, ResponseParam);

	if (bEncroached)
	{
		if (IsMovingOnGround())
		{
			// Something might be just barely overhead, try moving down closer to the floor to avoid it.
			const float MinFloorDist = UE_KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel,
				                                                    StandingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}
	return !bEncroached;
}

void UScreenCaptureCharacterMovementComponent::CrouchTick(float DeltaTime)
{
	if (!bIsRunningCrouchTickLogic) return;

	//store what's going to be old values
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();

	//figure out how much to expand player capsule this frame
	float DeltaToChange = DeltaTime * CrouchSpeed / FMath::Abs(TargetHalfHeight - StartHalfHeight);
	LerpAlphaValue += DeltaToChange;
	float NextHalfHeight = FMath::Lerp(StartHalfHeight, TargetHalfHeight, LerpAlphaValue);
	float ScaledHalfHeightAdjust = 0;
	
	if (TargetHalfHeight > StartHalfHeight) //uncrouching so need to run extra collision checks
	{
		const float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		const float HalfHeightAdjust = TargetHalfHeight -
			OldUnscaledHalfHeight;
		ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
		const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and standing size
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(
			SHRINK_HeightCustom,
			-SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		FVector StandingLocation = PawnLocation + FVector(
			0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
		bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel,
		                                                    StandingCapsuleShape, CapsuleParams, ResponseParam);

		if (bEncroached)
		{
			if (IsMovingOnGround())
			{
				// Something might be just barely overhead, try moving down closer to the floor to avoid it.
				const float MinFloorDist = UE_KINDA_SMALL_NUMBER * 10.f;
				if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
				{
					StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
					bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity,
					                                                    CollisionChannel,
					                                                    StandingCapsuleShape, CapsuleParams,
					                                                    ResponseParam);
				}
			}
		}

		if (bEncroached)
		{
			// CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
			LerpAlphaValue -= DeltaToChange; //revert lerp
			return;
		}
	}
	else
	{
		//just set capsule size
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, NextHalfHeight);
		float HalfHeightAdjust = (OldUnscaledHalfHeight - NextHalfHeight);
		ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	}

	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, NextHalfHeight, true);

	// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
	UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust),
	                                UpdatedComponent->GetComponentQuat(), true, nullptr,
	                                EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);


	//if "t" is bigger than 1 that means we've finished running the crouch/uncrouch movement and we just need to hold player capsule at that size now.
	if (LerpAlphaValue >= 1) bIsRunningCrouchTickLogic = false;
}
