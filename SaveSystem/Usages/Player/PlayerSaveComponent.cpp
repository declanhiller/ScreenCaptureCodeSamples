// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSaveComponent.h"

#include "CameraControllerComponent.h"
#include "CameraControllerComponentV2.h"
#include "GameFramework/Character.h"


// Sets default values for this component's properties
UPlayerSaveComponent::UPlayerSaveComponent()
{
}



bool UPlayerSaveComponent::GetSaveData(USavedData*& OutData)
{
	UPlayerSavedData* Data = NewObject<UPlayerSavedData>();
	
	if(!Data) return false;
	
	Data->Location = GetOwner()->GetActorLocation();
	Data->bHasCamera = GetOwner()->GetComponentByClass<UCameraControllerComponentV2>()->bIsCameraEnabled;
	Data->LookRotation = Cast<ACharacter>(GetOwner())->GetController()->GetControlRotation();

	OutData = Data;
	
	return true;
}

bool UPlayerSaveComponent::LoadSavedDataPostBegin(USavedData* LoadData)
{
	UPlayerSavedData* CastedData = Cast<UPlayerSavedData>(LoadData);
	if(!CastedData) return false;
	
	GetOwner()->SetActorLocation(CastedData->Location);
	GetOwner()->GetComponentByClass<UCameraControllerComponentV2>()->EnableCamera(CastedData->bHasCamera);
	Cast<ACharacter>(GetOwner())->GetController()->SetControlRotation(CastedData->LookRotation);
	
	return true;
}

