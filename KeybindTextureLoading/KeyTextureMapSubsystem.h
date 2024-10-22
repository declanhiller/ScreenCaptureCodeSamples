// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KeyTextureMapSubsystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SCREENCAPTURE_API UKeyTextureMapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	//visible for funsies!
	UPROPERTY(VisibleDefaultsOnly)
	TMap<FKey, UTexture*> Textures;

public:

	//get fallback texture if somebody tries to get a key with no recognized texture
	UPROPERTY(EditDefaultsOnly)
	UTexture* FallBackTexture;

	//type looking for
	UPROPERTY(EditDefaultsOnly)
	FPrimaryAssetType Type;

	//inits all the textures
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	//getter
	UFUNCTION(BlueprintCallable)
	UTexture* GetTextureForKey(FKey Key);
	
};
