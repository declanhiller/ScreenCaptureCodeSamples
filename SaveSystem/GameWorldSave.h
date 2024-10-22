// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveComponent.h"
#include "GameFramework/SaveGame.h"
#include "GameWorldSave.generated.h"

/**
 * Is THE global save data that saves all the other save datas like player save data,
 * object save data, progress save data, cutscene save data and so much more
 */
UCLASS()
class SCREENCAPTURE_API UGameWorldSave : public USaveGame
{
	GENERATED_BODY()

protected:

	//map with all saved data and their unique ids
	UPROPERTY()
	TMap<FGuid, FSavedSerializedData> SavedData;

public:

	//player save data
	UPROPERTY()
	FSavedSerializedData PlayerSavedData;

	//add data into the system
	void AddData(FGuid Key, FSavedSerializedData SaveData);

	TMap<FGuid, FSavedSerializedData> GetSavedData() const { return SavedData; }
};
