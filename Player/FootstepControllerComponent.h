// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "ScreenCapture/Player/ScreenCaptureCharacter.h"
#include "FootstepControllerComponent.generated.h"

USTRUCT(BlueprintType)
struct FMovementStateFootstepSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sound)
	TArray<USoundWave*> FootstepSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float MinVolumeMultiplier;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float MaxVolumeMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float MinPitchMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float MaxPitchMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Footsteps)
	float StepDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float AILoudnessValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float AIMaxRangeToHearFootstep;
	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class SCREENCAPTURE_API UFootstepControllerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFootstepControllerComponent();

private:
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	float CurrentStepDistance;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UScreenCaptureCharacterMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Creaking, meta = (AllowPrivateAccess="true"))
	TArray<USoundWave*> CreakingSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Creaking, meta = (AllowPrivateAccess="true"))
	float ChanceForCreak;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Creaking, meta = (AllowPrivateAccess="true"))
	float MinVolumeMultiplierForCreaking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Creaking, meta = (AllowPrivateAccess="true"))
	float MaxVolumeMultiplierForCreaking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Creaking, meta = (AllowPrivateAccess="true"))
	float MinPitchMultiplierForCreaking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Creaking, meta = (AllowPrivateAccess="true"))
	float MaxPitchMultiplierForCreaking;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void PlayFootstep();

	virtual void PlayCreak();

	virtual FMovementStateFootstepSettings GetCurrentFootstepSettings();

	UPROPERTY(Transient)
	bool bPlayedFirstFootstepSound;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category="Footstep Settings")
	FMovementStateFootstepSettings CrouchingSettings;

	UPROPERTY(EditAnywhere, Category="Footstep Settings")
	FMovementStateFootstepSettings WalkingSettings;

	UPROPERTY(EditAnywhere, Category="Footstep Settings")
	FMovementStateFootstepSettings RunningSettings;
};
