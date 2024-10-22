// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameWorldSave.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveManagerSubsystem.generated.h"

DECLARE_DELEGATE(FGameDataLoadedSignature)

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SCREENCAPTURE_API USaveManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USaveManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;
	
	UFUNCTION(BlueprintCallable)
	virtual bool SaveGame();

	UFUNCTION(BlueprintCallable)
	virtual void LoadGame();

	UFUNCTION(BlueprintCallable)
	virtual void HandleDeath();

	UFUNCTION(BlueprintCallable)
	virtual void LoadGameWorld();

	UFUNCTION(BlueprintCallable)
	virtual void DeleteCurrentSave();

	UFUNCTION(BlueprintCallable)
	virtual bool DoesSaveSlotExist();

	UFUNCTION()
	virtual void ActuallyLoadGameWorld();

	UFUNCTION(BlueprintCallable)
	virtual void ApplyDataToGameWorldPreBegin();

	UFUNCTION(BlueprintCallable)
	virtual void ApplyDataToGameWorldPostBegin();

	virtual bool WantsToApplySave();

	
	
protected:

	FGameDataLoadedSignature OnGameDataLoaded;
	
	UPROPERTY(Transient)
	UGameWorldSave* SavedData;
	
	UFUNCTION()
	virtual void ProcessLoadedData(const FString& SlotName, const int32 InUserIndex, USaveGame* InLoadedGameData);



	
};
