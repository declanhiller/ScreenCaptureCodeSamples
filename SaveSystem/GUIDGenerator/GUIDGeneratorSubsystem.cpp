// Fill out your copyright notice in the Description page of Project Settings.


#include "GUIDGeneratorSubsystem.h"

#include "Editor.h"
#include "ScreenCapture/SaveLoadSystem/SaveComponent.h"
#include "Subsystems/EditorActorSubsystem.h"

void UGUIDGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FEditorDelegates::OnNewActorsPlaced.AddUObject(this, &UGUIDGeneratorSubsystem::NewActorsPlaced);
	// FEditorDelegates::OnDuplicateActorsEnd.AddUObject(this, 
}

void UGUIDGeneratorSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGUIDGeneratorSubsystem::NewActorsPlaced(UObject* ObjToUse, const TArray<AActor*>& PlacedActors)
{
	for (AActor* PlacedActor : PlacedActors)
	{
		if(USaveComponent* SaveComponent = PlacedActor->GetComponentByClass<USaveComponent>())
		{
			SaveComponent->GeneratePersistentID(false);
		}
	}
}
