// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveComponent.h"
#include "StorableSaveComponent.generated.h"

UCLASS()
class UStorableSavedData : public USavedData
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	FVector Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	bool bStored;
	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SCREENCAPTURE_API UStorableSaveComponent : public USaveComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStorableSaveComponent();

	virtual bool GetSaveData(USavedData*& OutData) override;
	virtual bool LoadSavedDataPreBegin(USavedData* LoadedData) override;
	virtual bool LoadSavedDataPostBegin(USavedData* LoadData) override;
};
