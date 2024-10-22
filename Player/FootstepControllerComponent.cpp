// Fill out your copyright notice in the Description page of Project Settings.


#include "FootstepControllerComponent.h"

#include "ScreenCaptureCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Hearing.h"


// Sets default values for this component's properties
UFootstepControllerComponent::UFootstepControllerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//Crouching footsteps
	CrouchingSettings.MinVolumeMultiplier = 0.3f;
	CrouchingSettings.MaxVolumeMultiplier = 0.4f;
	CrouchingSettings.MinPitchMultiplier = 0.95f;
	CrouchingSettings.MaxPitchMultiplier = 1.05f;
	CrouchingSettings.StepDistance = 100;
	CrouchingSettings.AILoudnessValue = 0;
	CrouchingSettings.AIMaxRangeToHearFootstep = 0;

	//Walking footsteps
	WalkingSettings.MinVolumeMultiplier = 0.9f;
	WalkingSettings.MaxVolumeMultiplier = 1.1f;
	WalkingSettings.MinPitchMultiplier = 0.95f;
	WalkingSettings.MaxPitchMultiplier = 1.05f;
	WalkingSettings.StepDistance = 115;
	WalkingSettings.AILoudnessValue = 0.5f;
	WalkingSettings.AIMaxRangeToHearFootstep = 600;

	//Running footsteps
	RunningSettings.MinVolumeMultiplier = 0.9f;
	RunningSettings.MaxVolumeMultiplier = 1.1f;
	RunningSettings.MinPitchMultiplier = 0.95f;
	RunningSettings.MaxPitchMultiplier = 1.05f;
	RunningSettings.StepDistance = 125;
	RunningSettings.AILoudnessValue = 1;
	RunningSettings.AIMaxRangeToHearFootstep = 1000;

	//Creaking sounds
	ChanceForCreak = 0.02f;
	MinVolumeMultiplierForCreaking = 0.9f;
	MaxVolumeMultiplierForCreaking = 1.1f;
	MinPitchMultiplierForCreaking = 0.95f;
	MaxPitchMultiplierForCreaking = 0.95f;
}


// Called when the game starts
void UFootstepControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (UScreenCaptureCharacterMovementComponent* CharacterCast = GetOwner()->GetComponentByClass<
		UScreenCaptureCharacterMovementComponent>())
	{
		MovementComponent = CharacterCast;
	}
}

void UFootstepControllerComponent::PlayFootstep()
{
	FMovementStateFootstepSettings Settings = GetCurrentFootstepSettings();

	if (Settings.FootstepSounds.Num() <= 0) return;
	int32 FootstepIndex = FMath::RandRange(0, Settings.FootstepSounds.Num() - 1);
	float VolumeMultiplier = FMath::FRandRange(Settings.MinVolumeMultiplier,
	                                           Settings.MaxVolumeMultiplier);
	float PitchMultiplier = FMath::FRandRange(Settings.MinPitchMultiplier,
	                                          Settings.MaxPitchMultiplier);


	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Settings.FootstepSounds[FootstepIndex], GetComponentLocation(),
	                                      FRotator::ZeroRotator, VolumeMultiplier, PitchMultiplier);

	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetComponentLocation() + FVector(0,0,75), Settings.AILoudnessValue, GetOwner(), Settings.AIMaxRangeToHearFootstep);

	
	float RandomNumber = FMath::FRand();
	if (RandomNumber > ChanceForCreak) return;
}

void UFootstepControllerComponent::PlayCreak()
{
	int32 CreakingIndex = FMath::RandRange(0, CreakingSounds.Num() - 1);
	float CreakingVolumeMulti = FMath::FRandRange(MinVolumeMultiplierForCreaking,
	                                              MaxVolumeMultiplierForCreaking);
	float CreakLocalPitchMultiplier = FMath::FRandRange(MinPitchMultiplierForCreaking,
	                                                    MaxPitchMultiplierForCreaking);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), CreakingSounds[CreakingIndex], GetComponentLocation(),
	                                      FRotator::ZeroRotator, CreakingVolumeMulti, CreakLocalPitchMultiplier);
}

FMovementStateFootstepSettings UFootstepControllerComponent::GetCurrentFootstepSettings()
{
	if (MovementComponent->IsCrouching())
	{
		return CrouchingSettings;
	}

	if (MovementComponent->bIsRunning)
	{
		return RunningSettings;
	}

	return WalkingSettings;
}


// Called every frame
void UFootstepControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!MovementComponent->IsMovingOnGround())
	{
		CurrentStepDistance = 0;
		bPlayedFirstFootstepSound = false;
		return;
	}

	if (MovementComponent->Velocity.Length() < 1)
	{
		CurrentStepDistance = 0;
		bPlayedFirstFootstepSound = false;
	}

	if (!bPlayedFirstFootstepSound && MovementComponent->Velocity.Length() >= 1)
	{
		PlayFootstep();
		bPlayedFirstFootstepSound = true;
	}

	FVector CurrentVelocity = MovementComponent->Velocity;
	CurrentVelocity.Z = 0;
	CurrentStepDistance += CurrentVelocity.Length() * DeltaTime;
	if (CurrentStepDistance >= GetCurrentFootstepSettings().StepDistance)
	{
		PlayFootstep();
		CurrentStepDistance = 0;
	}
}
