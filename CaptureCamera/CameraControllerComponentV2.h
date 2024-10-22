// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraControllerComponent.h"
#include "CaptureModeValidator.h"
#include "Components/ActorComponent.h"
#include "ScreenCapture/Items/Components/StorableComponent.h"
#include "CameraControllerComponentV2.generated.h"

//message enums
UENUM(BlueprintType)
enum ECaptureMessage
{
	OutsideOfDistance,
	LowVisibility,
	NoFocusObject,
	Cancelled,
	NoMoreSpace,
	Captured,
	InsideOfObject
};

UENUM(BlueprintType)
enum EPlaceMessage { InvalidPlacement, ValidPlacement };

UENUM(BlueprintType)
enum ELibraryMessage { SuccessfulySelected, CantPlace };

//struct that represents a file slot in the camera
USTRUCT(BlueprintType)
struct FStorageSlot
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	UStorableComponent* StoredObject = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCaptureSegmentCompletedSignature, int32, CompleteSegmentNumber);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCaptureMessageSignature, TEnumAsByte<ECaptureMessage>, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlaceMessageSignature, TEnumAsByte<EPlaceMessage>, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLibraryMessageSignature, TEnumAsByte<ELibraryMessage>, Message);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectStoredSignature, FStorageSlot, StoredSlot);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectPlacedSignature, FStorageSlot, PlacedSlot);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSwatcheltCaptured); //hard coding stuff, what's that


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SCREENCAPTURE_API UCameraControllerComponentV2 : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCameraControllerComponentV2();

	//values used as a cheap determination if target is within frame before running more complex checks
	UPROPERTY(EditDefaultsOnly)
	float ConeAngle;
	UPROPERTY()
	float ConeDot;

	//max distance that a camera can detect an object
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxDistanceForCapturing;

	//channel that the camera can collide against when line casting against geometry
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ETraceTypeQuery> VisibilityTraceType;

	//channel used when detecting if an object is in frame
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ETraceTypeQuery> TraceTypeForFrameDetection;

	//channel used when detecting which objects can/can't overlap with objects that are being placed
	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesForPlaceOverlap;

	//amount of objects that can be stored in the camera
	UPROPERTY(EditAnywhere, Category = "Store Mode")
	int32 CameraObjectCapacity;

	//exact object type that can be stored in camera
	UPROPERTY(EditAnywhere, Category = "Store Mode")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesOfStorableActors;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> PreviewSceneCaptureActorType;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	virtual void SetRecordingCameraComponent(USceneCaptureComponent2D* SceneComponent)
	{
		ActualRecordingCameraComponent = SceneComponent;
	}

	UFUNCTION(BlueprintCallable)
	void SetCaptureValidator(UCaptureModeValidator* CaptureModeValidator) { CaptureValidator = CaptureModeValidator; }

	UFUNCTION(BlueprintCallable)
	virtual void SetMeshComponent(USceneComponent* SceneComponent) { MeshComponent = SceneComponent; }

protected:
	//Object references this component needs to function
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient)
	USceneComponent* MeshComponent;

	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient)
	USceneCaptureComponent2D* ActualRecordingCameraComponent; //Actual component that is getting the stuff to render

	//validator component that does the more expensive and accurate check to see if something is in frame
	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient)
	UCaptureModeValidator* CaptureValidator;

public:
	//Camera Controller Events
	UPROPERTY(BlueprintAssignable)
	FOnNewCameraMode OnNewCameraMode;

	UPROPERTY(BlueprintAssignable)
	FOnNewCameraState OnNewCameraState;

	UPROPERTY(BlueprintAssignable)
	FObjectStoredSignature OnObjectStored;

	UPROPERTY(BlueprintAssignable)
	FOnPhotoTaken OnPhotoTaken;

	UPROPERTY(BlueprintAssignable)
	FOnCaptureStatusUpdate OnCaptureStatusUpdate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FObjectPlacedSignature OnObjectPlaced;

	UPROPERTY(BlueprintAssignable)
	FOnSelectionChanged OnSelectionChanged;

	UPROPERTY(BlueprintAssignable)
	FOnNewMessage OnNewMessage;

	UPROPERTY(BlueprintAssignable)
	FCaptureSegmentCompletedSignature OnACaptureSegmentCompleted;

	UPROPERTY(BlueprintAssignable)
	FCaptureMessageSignature OnNewCaptureMessage;

	UPROPERTY(BlueprintAssignable)
	FLibraryMessageSignature OnLibraryMessage;

	UPROPERTY(BlueprintAssignable)
	FSwatcheltCaptured OnSwatchletCaptured;

public:
	//Camera State Logic

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<ECameraState> CurrentCameraState;

	UFUNCTION(BlueprintCallable)
	virtual void SwitchCameraState(TEnumAsByte<ECameraState> NewState);

	UPROPERTY(BlueprintReadWrite)
	bool bPlayingEquipAnimation;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsCameraEnabled;

	UFUNCTION(BlueprintCallable)
	void EnableCamera(bool bValue) { bIsCameraEnabled = bValue; }

protected:
	//initiation and teardown of specific states
	virtual void HandleStateTeardown(ECameraState OldState);
	virtual void HandleStateInit(ECameraState NewState);

public:
	//Camera Mode Logic
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<ECameraMode> CurrentCameraMode;

public:
	//Generic Camera use Logic

	UFUNCTION(BlueprintCallable)
	virtual void StartUseCamera();

	//basically cancel camera use
	UFUNCTION(BlueprintCallable)
	virtual void EndUseCamera();

	//put in so swatchlet can cancel camera use with "special" conditions
	//this is kinda jank and stupid tbh
	UFUNCTION(BlueprintCallable)
	virtual void CancelCameraUse();

	//go back to camera
	UFUNCTION(BlueprintCallable)
	virtual void GoBack();

	//open up library
	UFUNCTION(BlueprintCallable)
	virtual void GoToLibrary();

	//storage slots in library
	UPROPERTY(BlueprintReadWrite)
	TArray<FStorageSlot> StorageSlots;

	//current slot that is being selected
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentSlotIndex;

protected:
	//start a capture of an object
	virtual void StartCapture();

	//end the capture of an object with the option of it ending successfully
	virtual void EndCaptureCharge();

	//full on cancel capture charge and fail the charge
	virtual void CancelCapture();

	//end the capture successfully
	virtual void EndCaptureSuccessfully();

	//helper function that will check using the capture validator if it's still working and output a capture error depending on issue
	virtual bool IsFocusedStorableStillInFrame(TEnumAsByte<ECaptureMessage>& CaptureError);

	//check the current segment in the progress bar to see if capture is still valid
	virtual void CheckCurrentCaptureSegment();

	//make the current capture hold as failed
	virtual void MarkCaptureAsFailed(TEnumAsByte<ECaptureMessage> Error);

	//mark the capture progress as finished and ready for the player to release the button to conclude capture
	virtual void MarkCaptureAsReadyToFinish();

	//run capture tick logic
	virtual void CaptureTick(float DeltaTime);

	FTimerHandle CaptureHoldTimer;
	bool bIsCameraChargingUp;
	bool bCameraReadyToEndSuccessfulCaptureSequence;

	//storables in the last frame
	UPROPERTY()
	TArray<UStorableComponent*> LastFrameValidStorables;

public:
	//current storable that is closest to player LOS
	UPROPERTY(BlueprintReadOnly)
	UStorableComponent* FocusedStorable;

	//how many segments have been completed in current capture progress
	UPROPERTY(BlueprintReadOnly)
	int32 NumberOfCaptureSegmentsCompleted;

	//put an object inside the camera
	UFUNCTION(BlueprintCallable)
	virtual void StoreObject(UStorableComponent* Storable);

public:
	//Photo Library Logic

	UFUNCTION(BlueprintCallable)
	virtual void ScrollUpInObjectStorage();

	UFUNCTION(BlueprintCallable)
	virtual void ScrollDownInObjectStorage();

	virtual void SwitchToSlot(int32 Index);

public:
	//Camera Placing Logic

	UFUNCTION(BlueprintCallable)
	virtual void SelectPhotoForPlacing();

	virtual void HandlePlacingTick(float DeltaTime);

	virtual void PlaceModeInit();
	virtual void PlaceModeTeardown();

	//take in input to determine if player is rotating or not rotating objects and in which direction
	
	virtual void MarkRotateRight()
	{
		bIsRotatingObjectRight = true;
	}

	virtual void UnmarkRotateRight() { bIsRotatingObjectRight = false; }

	virtual void MarkRotateLeft() { bIsRotatingObjectLeft = true; }
	virtual void UnmarkRotateLeft() { bIsRotatingObjectLeft = false; }


	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EPlaceMessage> PlaceMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient)
	float ObjectRotationSpeed;

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsRotatingObjectRight;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsRotatingObjectLeft;

public:
	//what percentage are we at in total charge
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetElapsedPercentageOfChargeTime()
	{
		if (!FocusedStorable) return 0;
		float PercentageOfSegmentsCompleted = (float)NumberOfCaptureSegmentsCompleted / FocusedStorable->
			NumberOfCaptureSegments;
		return PercentageOfSegmentsCompleted + (GetElapsedPercentageOnCurrentSegment() / FocusedStorable->
			NumberOfCaptureSegments);
	}

	//what percentage are we at in current segment
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetElapsedPercentageOnCurrentSegment()
	{
		if (!FocusedStorable) return 0;
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		if (!TimerManager.TimerExists(CaptureHoldTimer)) return 0;
		return TimerManager.GetTimerElapsed(CaptureHoldTimer) / FocusedStorable->TimeOfACaptureSegment;
	}

	//get the dot product from a specific target
	float GetDotProductForTarget(USceneComponent* TargetComponent) const;

	//check if a target is in frame using a cheap dot product check instead of the capture validator
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsTargetWithinFrame(USceneComponent* TargetComponent) const;
};
