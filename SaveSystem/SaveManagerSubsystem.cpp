// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveManagerSubsystem.h"

#include "GameWorldSave.h"
#include "SaveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ScreenCapture/Player/ScreenCaptureCharacter.h"
#include "ScreenCapture/Player/Components/PlayerSaveComponent.h"

USaveManagerSubsystem::USaveManagerSubsystem()
{
	SaveSlotName = "TestUser";
	UserIndex = 0;
}

void USaveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

bool USaveManagerSubsystem::SaveGame()
{
	UGameWorldSave* GameSaveObject = Cast<UGameWorldSave>(
		UGameplayStatics::CreateSaveGameObject(UGameWorldSave::StaticClass()));
	if (!GameSaveObject) return false; //wasn't able to create a valid save game object

	UWorld* World = GetWorld();
	if(!World) return false; //no world to get game save lmao

	for ( TObjectIterator<USaveComponent> SavableObj; SavableObj; ++SavableObj )
	{
		if(SavableObj->IsTemplate()) continue;
		if(SavableObj->GetWorld() != GetWorld()) continue;

		if(SavableObj->IsA(UPlayerSaveComponent::StaticClass()))
		{
			USavedData* Data;
			if(!SavableObj->GetSaveData(Data)) continue;
			GameSaveObject->PlayerSavedData = Data->GetSerializedData();
		}
		
		if(!SavableObj->PersistentID.IsValid()) continue;
		USavedData* Data;
		if(!SavableObj->GetSaveData(Data)) continue;
		GameSaveObject->AddData(SavableObj->PersistentID, Data->GetSerializedData());
	}
	
	UGameplayStatics::AsyncSaveGameToSlot(GameSaveObject, SaveSlotName, UserIndex);
	return true;
}

void USaveManagerSubsystem::LoadGame()
{
	// Set up the delegate.
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	// USomeUObjectClass::LoadGameDelegateFunction is a void function that takes the following parameters: const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData
	LoadedDelegate.BindUObject(this, &USaveManagerSubsystem::ProcessLoadedData);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, UserIndex, LoadedDelegate);
}

void USaveManagerSubsystem::ProcessLoadedData(const FString& SlotName, const int32 InUserIndex, USaveGame* InLoadedGameData)
{
	UGameWorldSave* LoadedGameData = Cast<UGameWorldSave>(InLoadedGameData);

	if(!LoadedGameData) return;

	SavedData = LoadedGameData;

	OnGameDataLoaded.ExecuteIfBound();
	OnGameDataLoaded.Unbind();
	
	// for ( TObjectIterator<USaveComponent> SavableObj; SavableObj; ++SavableObj )
	// {
	// 	if(SavableObj->IsTemplate()) continue;
	// 	if(SavableObj->GetWorld() != GetWorld()) continue; //make sure we are not editing editor version of component
	// 	if(!LoadedGameData->GetSavedData().Contains(SavableObj->PersistentID)) continue;
	// 	FSavedSerializedData SaveData = *LoadedGameData->GetSavedData().Find(SavableObj->PersistentID);
	// 	USavedData* Data = NewObject<USavedData>(GetTransientPackage(), SaveData.ObjectClass);
	// 	Data->ApplySerializedData(SaveData);
	// 	SavableObj->LoadSavedData(Data);
	// }
	
}

void USaveManagerSubsystem::HandleDeath()
{
	UGameplayStatics::OpenLevel(GetWorld(), "DeathLevel");
}

void USaveManagerSubsystem::LoadGameWorld()
{
	if(DoesSaveSlotExist())
	{
		OnGameDataLoaded.BindUObject(this, &USaveManagerSubsystem::ActuallyLoadGameWorld);
		LoadGame();
	} else
	{
		UGameplayStatics::OpenLevel(GetWorld(), "MainMap");
	}

}

void USaveManagerSubsystem::DeleteCurrentSave()
{
	if(UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SaveSlotName, UserIndex);
	}
}

bool USaveManagerSubsystem::DoesSaveSlotExist()
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

void USaveManagerSubsystem::ActuallyLoadGameWorld()
{
	UGameplayStatics::OpenLevel(GetWorld(), "MainMap");
}

void USaveManagerSubsystem::ApplyDataToGameWorldPreBegin()
{

	if(!SavedData) return;
	
	for ( TObjectIterator<USaveComponent> SavableObj; SavableObj; ++SavableObj )
	{


		
		if(SavableObj->IsTemplate()) continue;
		if(SavableObj->GetWorld() != GetWorld()) continue; //make sure we are not editing editor version of component

		if(SavableObj->IsA(UPlayerSaveComponent::StaticClass()))
		{
			FSavedSerializedData SaveData = SavedData->PlayerSavedData;
			USavedData* Data = NewObject<USavedData>(GetTransientPackage(), SaveData.ObjectClass);
			Data->ApplySerializedData(SaveData);
			SavableObj->LoadSavedDataPreBegin(Data);
		}
		
		if(!SavedData->GetSavedData().Contains(SavableObj->PersistentID)) continue;
		FSavedSerializedData SaveData = *SavedData->GetSavedData().Find(SavableObj->PersistentID);
		USavedData* Data = NewObject<USavedData>(GetTransientPackage(), SaveData.ObjectClass);
		Data->ApplySerializedData(SaveData);
		SavableObj->LoadSavedDataPreBegin(Data);
	}
}

void USaveManagerSubsystem::ApplyDataToGameWorldPostBegin()
{
	if(!SavedData) return;
	
	for ( TObjectIterator<USaveComponent> SavableObj; SavableObj; ++SavableObj )
	{
		if(SavableObj->IsTemplate()) continue;
		if(SavableObj->GetWorld() != GetWorld()) continue; //make sure we are not editing editor version of component

		if(SavableObj->IsA(UPlayerSaveComponent::StaticClass()))
		{
			FSavedSerializedData SaveData = SavedData->PlayerSavedData;
			USavedData* Data = NewObject<USavedData>(GetTransientPackage(), SaveData.ObjectClass);
			Data->ApplySerializedData(SaveData);
			SavableObj->LoadSavedDataPostBegin(Data);
		}
		
		if(!SavedData->GetSavedData().Contains(SavableObj->PersistentID)) continue;
		FSavedSerializedData SaveData = *SavedData->GetSavedData().Find(SavableObj->PersistentID);
		USavedData* Data = NewObject<USavedData>(GetTransientPackage(), SaveData.ObjectClass);
		Data->ApplySerializedData(SaveData);
		SavableObj->LoadSavedDataPostBegin(Data);
	}

	SavedData = nullptr;

}

bool USaveManagerSubsystem::WantsToApplySave()
{
	return SavedData != nullptr;
}

