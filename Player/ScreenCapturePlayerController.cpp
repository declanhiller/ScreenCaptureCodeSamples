// Fill out your copyright notice in the Description page of Project Settings.


#include "ScreenCapturePlayerController.h"

#include "CameraEffects/LandCameraModifier.h"
#include "CameraEffects/LandCameraModifierV2.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "ScreenCapture/UI/ScreenCaptureGameUserSettings.h"


AScreenCapturePlayerController::AScreenCapturePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	MinimumSpeedToActivateWalkCameraShake = 50;
	MinimumSpeedToActivateRunCameraShake = 200;
	bIsCameraRotationEnabled = true;
}

void AScreenCapturePlayerController::BeginPlay()
{
	Super::BeginPlay();

	Settings = Cast<UScreenCaptureGameUserSettings>(UGameUserSettings::GetGameUserSettings());
	
	ScreenCaptureCharacter = Cast<AScreenCaptureCharacter>(GetPawn());

	if(!ScreenCaptureCharacter) return;
	
	ScreenCaptureCharacter->OnLanded.AddUObject(this, &AScreenCapturePlayerController::RunLandCameraModifier);
	if(LandingCameraModifierClass)
	{
		LandCameraModifier = Cast<ULandCameraModifierV2>(PlayerCameraManager->AddNewCameraModifier(LandingCameraModifierClass));
	} else
	{
		LandCameraModifier = Cast<ULandCameraModifierV2>(PlayerCameraManager->AddNewCameraModifier(ULandCameraModifierV2::StaticClass()));
	}
	LandCameraModifier->FinishedDelegate.AddUObject(this, &AScreenCapturePlayerController::FinishedLandingCallback);
}

void AScreenCapturePlayerController::AddPitchInput(float Val)
{
	if(!bIsCameraRotationEnabled) return;
	Super::AddPitchInput(Val);
}

void AScreenCapturePlayerController::AddYawInput(float Val)
{
	if(!bIsCameraRotationEnabled) return;
	Super::AddYawInput(Val);
}

void AScreenCapturePlayerController::RunLandCameraModifier(FVector LandVelocity)
{
	if(LandCameraModifier->Land(LandVelocity))
	{
		bIsCameraRotationEnabled = false;
	}
}

void AScreenCapturePlayerController::FinishedLandingCallback()
{
	if(!ScreenCaptureCharacter) return;
	bIsCameraRotationEnabled = true;
	ScreenCaptureCharacter->FinishedLanding();
	ScreenCaptureCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AScreenCapturePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(!Settings->GetHeadBob()) return;
	if(!GetPawn()) return;
	FVector PawnVelocity = GetPawn()->GetVelocity();
	PawnVelocity.Z = 0;
	float Speed = PawnVelocity.Length();
	if(Speed > MinimumSpeedToActivateRunCameraShake)
	{
		ClientStartCameraShake(RunCameraShake);
		ClientStopCameraShake(WalkCameraShake, false);
	} else if(Speed > MinimumSpeedToActivateWalkCameraShake)
	{
		ClientStartCameraShake(WalkCameraShake);
		ClientStopCameraShake(RunCameraShake, false);
	}
	else
	{
		ClientStopCameraShake(RunCameraShake, false);
		ClientStopCameraShake(WalkCameraShake, false);
	}
	// if(ScreenCaptureCharacter)
	// {
	// 	ScreenCaptureCharacter->SetCurrentPlayerCameraManagerLocation(PlayerCameraManager->GetCameraLocation());
	// }
}
