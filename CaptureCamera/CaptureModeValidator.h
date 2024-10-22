// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CaptureModeValidator.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SCREENCAPTURE_API UCaptureModeValidator : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCaptureModeValidator();


public:

	//check if storable is actually within frame of camera view
	virtual bool ValidateStorableInFrame();

};
