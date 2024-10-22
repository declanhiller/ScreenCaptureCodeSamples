// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ScreenCaptureCharacterMovementComponent.generated.h"

/**
 * 
 */


DECLARE_DELEGATE(FOnFootstep)

UCLASS()
class SCREENCAPTURE_API UScreenCaptureCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()


public:

	UPROPERTY(Category="Character Movement: Running", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	float RunSpeed;
	
	UPROPERTY(Category="Character Movement: Running", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	float CrouchSpeed;

	UPROPERTY(Category="Character Movement: Running", EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	float StunnedSpeed;
	
public:

	UScreenCaptureCharacterMovementComponent();

	virtual void BeginPlay() override;
	
	UPROPERTY(Category="Character Movement: Running", BlueprintReadOnly)
	bool bIsRunning;

	UPROPERTY(Category = "Character Movement: Running", BlueprintReadWrite)
	bool bWantsToRun;
	
	UPROPERTY()
	float StoredWalkSpeed;
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	//update the state of the character before moving
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	//Stun the player for the inputted amount of time
	UFUNCTION(BlueprintCallable)
	void Stun(float TimeToStun);

	//Unstun the player, can be called manually or is called after delay with Stun()
	UFUNCTION(BlueprintCallable)
	void UnStun();

	
	virtual void Crouch(bool bClientSimulation) override;
	virtual void UnCrouch(bool bClientSimulation) override;

	//Does player have space above head to uncrouch
	virtual bool CanUnCrouch();
protected:
	//tick crouch for actual crouch movement
	virtual void CrouchTick(float DeltaTime);

	UPROPERTY(Category = "Character Movement: Running", BlueprintReadWrite)
	bool bStunned;
	
	//for crouching and uncrouching
	UPROPERTY(Transient)
	bool bIsRunningCrouchTickLogic;

	//value used to track the position that player is in while crouching/uncrouching
	UPROPERTY(Transient)
	float LerpAlphaValue;

	//Start height of the player capsule for crouching/uncrouching
	UPROPERTY(Transient)
	float StartHalfHeight;

	//target height of the player capsule for crouching/uncrouching
	UPROPERTY(Transient)
	float TargetHalfHeight;

	//time handle that actually tracks amount of time to stay stunned
	FTimerHandle StunTimerHandle;
	
};
