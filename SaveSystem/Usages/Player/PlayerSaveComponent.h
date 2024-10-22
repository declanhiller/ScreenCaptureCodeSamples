// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScreenCapture/SaveLoadSystem/SaveComponent.h"
#include "PlayerSaveComponent.generated.h"

UCLASS()
class UPlayerSavedData : public USavedData
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	FVector Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	FRotator LookRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	bool bHasCamera;
	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SCREENCAPTURE_API UPlayerSaveComponent : public USaveComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerSaveComponent();

	virtual bool GetSaveData(USavedData*& OutData) override;
	virtual bool LoadSavedDataPostBegin(USavedData* LoadData) override;
};
