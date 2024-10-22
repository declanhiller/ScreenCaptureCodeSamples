// Fill out your copyright notice in the Description page of Project Settings.


#include "GameWorldSave.h"

void UGameWorldSave::AddData(FGuid Key, FSavedSerializedData SaveData)
{
	SavedData.Add(Key, SaveData);
}
