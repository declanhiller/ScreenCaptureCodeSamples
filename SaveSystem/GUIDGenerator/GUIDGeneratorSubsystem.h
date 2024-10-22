// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "GUIDGeneratorSubsystem.generated.h"

class UEditorActorSubsystem;
/**
 * 
 */
UCLASS()
class EDITORGUIDGENERATOR_API UGUIDGeneratorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	UFUNCTION()
	virtual void NewActorsPlaced(UObject* ObjToUse, const TArray<AActor*>& PlacedActors);
	//
	// UFUNCTION()
	// virtual void NewActorsDuplicated(UObject* ObjToUse, const TArray<AActor*>& DuplicatedActors);
	
};
