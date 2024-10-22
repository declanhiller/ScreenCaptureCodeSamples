// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScreenCaptureCharacter.h"
#include "GameFramework/PlayerController.h"
#include "ScreenCapturePlayerController.generated.h"

class ULandCameraModifierV2;
/**
 * 
 */
UCLASS()
class SCREENCAPTURE_API AScreenCapturePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AScreenCapturePlayerController();

protected:
	virtual void BeginPlay() override;

private:

	//character
	UPROPERTY(BlueprintReadOnly, meta =(AllowPrivateAccess="true"))
	TObjectPtr<AScreenCaptureCharacter> ScreenCaptureCharacter;

	UPROPERTY(Transient)
	class UScreenCaptureGameUserSettings* Settings;
	
	//Head bobbing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tweening, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> WalkCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tweening, meta = (AllowPrivateAccess = "true"))
	float MinimumSpeedToActivateWalkCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tweening, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> RunCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tweening, meta = (AllowPrivateAccess = "true"))
	float MinimumSpeedToActivateRunCameraShake;


	UPROPERTY(EditAnywhere, Category = Landing, meta = (AllowPrivateAccess="true"))
	TSubclassOf<ULandCameraModifierV2> LandingCameraModifierClass;
	
	UPROPERTY(Transient)
	ULandCameraModifierV2* LandCameraModifier;

public:
	virtual void AddPitchInput(float Val) override;
	virtual void AddYawInput(float Val) override;

private:
	void RunLandCameraModifier(FVector LandVelocity);

	void FinishedLandingCallback();
	
	

public:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadWrite)
	bool bIsCameraRotationEnabled;
};
