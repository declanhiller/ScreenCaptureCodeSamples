// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveComponent.h"

#include "SavedData.h"


// Sets default values for this component's properties
USaveComponent::USaveComponent()
{
	

	Priority = 10;
}


bool USaveComponent::GeneratePersistentID(bool bForceGeneration)
{
	if(!PersistentID.IsValid() || bForceGeneration)
	{
		PersistentID = FGuid::NewGuid();
		return true;
	}
	return false;
}


void USaveComponent::PostEditImport()
{
	GeneratePersistentID(true);
}

bool USaveComponent::GetSaveData(USavedData*& OutData)
{
	return GetSaveData_Blueprint(OutData);
}

bool USaveComponent::LoadSavedDataPreBegin(USavedData* LoadData)
{
	return LoadSavedData_Blueprint(LoadData);
}

bool USaveComponent::LoadSavedDataPostBegin(USavedData* LoadData)
{
	return LoadSavedDataPostBegin_Blueprint(LoadData);
}
