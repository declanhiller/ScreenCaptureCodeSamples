// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureModeValidator.h"

#include "Engine/TextureRenderTarget2D.h"


// Sets default values for this component's properties
UCaptureModeValidator::UCaptureModeValidator()
{
	//setup camera component correctly
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = true;
}



bool UCaptureModeValidator::ValidateStorableInFrame()
{
	//actually process texture
	CaptureScene();

	//basicaly check the percentage of white pixels to black pixels based on a texture target
	TArray<FColor> SurfData;
	FRenderTarget *RenderTarget = TextureTarget->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(SurfData);
	// Index formula
	int WhiteCounter = 0;
	for(int i = 0; i < SurfData.Num(); i++)
	{
		if(SurfData[i].R > 100 || SurfData[i].G > 100 || SurfData[i].B > 100)
		{
			WhiteCounter++;
		}
	}

	float Ratio = static_cast<float>(WhiteCounter)/SurfData.Num();
	
	return Ratio > 0.005f;
	

	
}

