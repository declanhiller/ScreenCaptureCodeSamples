// Fill out your copyright notice in the Description page of Project Settings.


#include "StorableSaveComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "ScreenCapture/Items/Components/StorableComponent.h"
#include "ScreenCapture/Player/Components/CameraControllerComponent.h"
#include "ScreenCapture/Player/Components/CameraControllerComponentV2.h"


// Sets default values for this component's properties
UStorableSaveComponent::UStorableSaveComponent()
{

}

bool UStorableSaveComponent::GetSaveData(USavedData*& OutData)
{
	//construct save data that will be stored
	UStorableSavedData* Data = NewObject<UStorableSavedData>();

	//make sure creation was successful
	if (!Data) return false;

	//store data
	Data->Location = GetOwner()->GetActorLocation();
	Data->bStored = GetOwner()->GetComponentByClass<UStorableComponent>()->bStored;
	OutData = Data;

	return true;
}

bool UStorableSaveComponent::LoadSavedDataPreBegin(USavedData* LoadedData)
{
	UStorableSavedData* CastedData = Cast<UStorableSavedData>(LoadedData);
	if (!CastedData) return false;
	
	GetOwner()->SetActorLocation(CastedData->Location);

	return true;
}

bool UStorableSaveComponent::LoadSavedDataPostBegin(USavedData* LoadData)
{
	UStorableSavedData* CastedData = Cast<UStorableSavedData>(LoadData);
	if (!CastedData) return false;

	if (CastedData->bStored)
	{
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetComponentByClass<UCameraControllerComponentV2>()->
		                                                     StoreObject(
			                                                     GetOwner()->GetComponentByClass<UStorableComponent>());
	}

	return true;
}
