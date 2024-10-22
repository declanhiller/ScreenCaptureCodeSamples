// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SavedData.h"
#include "Components/ActorComponent.h"
#include "SaveComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActorLoadedFromSaveSignature);

UCLASS(ClassGroup=(SaveLoad), meta=(BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class SCREENCAPTURE_API USaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USaveComponent();

	//unqiue persistent id that is used to identify which save data goes to which object
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Save)
	FGuid PersistentID;

	//generate a id that can be used for saving
	virtual bool GeneratePersistentID(bool bForceGeneration);

	UPROPERTY(EditAnywhere, Category = Save)
	float Priority; //priority to load in objects... higher means object will be loaded in first, lower means object will be loaded in last

	UPROPERTY(BlueprintAssignable, Category = Save)
	FOnActorLoadedFromSaveSignature OnActorLoadedFromSave;

	virtual void PostEditImport() override;
	
	virtual bool GetSaveData(USavedData*& OutData);

	UFUNCTION(BlueprintImplementableEvent)
	bool GetSaveData_Blueprint(USavedData*& OutData);

	virtual bool LoadSavedDataPreBegin(USavedData* LoadData);

	virtual bool LoadSavedDataPostBegin(USavedData* LoadData);


	UFUNCTION(BlueprintImplementableEvent)
	bool LoadSavedData_Blueprint(USavedData* OutData);

	UFUNCTION(BlueprintImplementableEvent)
	bool LoadSavedDataPostBegin_Blueprint(USavedData* OutData);
};

