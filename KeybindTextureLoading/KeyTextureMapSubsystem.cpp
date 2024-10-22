// Fill out your copyright notice in the Description page of Project Settings.


#include "KeyTextureMapSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"

void UKeyTextureMapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//load up asset manager stuff
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> OutAssetDatas;
	
	//get all assets in path
	AssetRegistryModule.Get().GetAssetsByPath("/Game/Art/KeyIcons", OutAssetDatas, true);

	//get all Unreal recognized keys
	TArray<FKey> OutKeys;
	EKeys::GetAllKeys(OutKeys);
	//loop through all recognized keys
	for (FKey Key : OutKeys)
	{
		FName KeyName = Key.GetFName();

		//see if texture exists for key name
		FAssetData* Data = OutAssetDatas.FindByPredicate([KeyName](FAssetData Data) {return Data.AssetName.IsEqual(KeyName);});
		if(!Data) continue;
		
		//if the data exists for the key then get the texture
		UTexture* Texture = Cast<UTexture>(Data->GetAsset());

		//just make sure texture exists which it should in 100% but Unreal unreals sometimes
		if(!Texture) continue;

		//add texture to map
		Textures.Add(Key, Texture);
		
	}
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString("Compiled Key Texture Map"));
}

UTexture* UKeyTextureMapSubsystem::GetTextureForKey(FKey Key)
{
	//access texture for key
	if(!Textures.Contains(Key)) return FallBackTexture;
	UTexture* Texture = *Textures.Find(Key);
	if(!Texture)
	{
		return FallBackTexture;
	}
	return Texture;
}
