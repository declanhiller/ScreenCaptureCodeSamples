// Fill out your copyright notice in the Description page of Project Settings.


#include "SavedData.h"

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

FSavedSerializedData USavedData::GetSerializedData()
{
	TArray<uint8> SerializedData;
	FMemoryWriter Writer(SerializedData, true);
	FObjectAndNameAsStringProxyArchive Archive(Writer, true);
	Writer.SetIsSaving(true);
	Serialize(Archive);


	FSavedSerializedData OutData;
	OutData.ObjectClass = this->GetClass();
	OutData.Data = SerializedData;
	return OutData;
}

bool USavedData::ApplySerializedData(FSavedSerializedData SerializedData)
{
	if (SerializedData.Data.Num() <= 0)
	{
		return false;
	}
     
	FMemoryReader ActorReader(SerializedData.Data, true);
	FObjectAndNameAsStringProxyArchive Archive(ActorReader, true);
	ActorReader.SetIsLoading(true);
	Serialize(Archive);

	return true;
}
