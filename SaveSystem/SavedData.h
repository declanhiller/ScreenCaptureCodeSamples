// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SavedData.generated.h"

class USavedData;

USTRUCT(BlueprintType)
struct FSavedSerializedData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Serialization")
	TSubclassOf<USavedData> ObjectClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "Serialization")
	TArray<uint8> Data;
    
	friend FArchive& operator << (FArchive& Ar, FSavedSerializedData& Object)
	{
		Ar << Object.Data;
		return Ar;
	}
	
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SCREENCAPTURE_API USavedData : public UObject
{
	GENERATED_BODY()

public:
	
	FSavedSerializedData GetSerializedData();

	bool ApplySerializedData(FSavedSerializedData SerializedData);
	
};
