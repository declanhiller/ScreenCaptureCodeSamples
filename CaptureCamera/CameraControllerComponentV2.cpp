// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraControllerComponentV2.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ScreenCapture/Items/Components/StorableComponent.h"
#include "ScreenCapture/Lifeforms/Swatchlet/Swatchlet.h"


// Sets default values for this component's properties
UCameraControllerComponentV2::UCameraControllerComponentV2()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	//setup default values
	ConeAngle = 30;
	MaxDistanceForCapturing = 500;
	CameraObjectCapacity = 6;
	ObjectRotationSpeed = 100;
}


// Called when the game starts
void UCameraControllerComponentV2::BeginPlay()
{
	Super::BeginPlay();

	// ...

	//caluclate cone dot so we don't have to run a Cos calculation every time we want to compare dot products
	ConeDot = FMath::Cos(FMath::DegreesToRadians(ConeAngle));
}


// Called every frame
void UCameraControllerComponentV2::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//tick correct functionality determined by state
	switch (CurrentCameraMode)
	{
	case CaptureMode:
		CaptureTick(DeltaTime);
		break;
	case Place:
		HandlePlacingTick(DeltaTime);
		break;
	}
}

void UCameraControllerComponentV2::SwitchCameraState(TEnumAsByte<ECameraState> NewState)
{
	//make sure that camera state can actually be switched
	if (bPlayingEquipAnimation) return;
	if (!bIsCameraEnabled) return;
	//make sure that we aren't just trying to switch to the existing state
	if (CurrentCameraState == NewState) return;
	//teardown old state
	HandleStateTeardown(CurrentCameraState);
	//init new state
	HandleStateInit(NewState);
	//broadcast that state has been set
	OnNewCameraState.Broadcast(NewState);
	
	CurrentCameraState = NewState;
}

void UCameraControllerComponentV2::HandleStateTeardown(ECameraState OldState)
{
	if (OldState == Stowed)
	{
		//handle mode init
		if (CurrentCameraMode == Place)
		{
			PlaceModeInit();
		}
	}
}

void UCameraControllerComponentV2::HandleStateInit(ECameraState NewState)
{
	if (NewState == Stowed)
	{
		//Handle mode teardown
		if (CurrentCameraMode == Place)
		{
			PlaceModeTeardown();
		}
	}
}

void UCameraControllerComponentV2::StartUseCamera()
{
	//just start a capture
	if (CurrentCameraMode == CaptureMode)
	{
		StartCapture();
	}
}

void UCameraControllerComponentV2::EndUseCamera()
{
	if (CurrentCameraMode == CaptureMode)
	{
		//try to end capture charge, we let that function figure out what to do next
		EndCaptureCharge();
	}
	else if (CurrentCameraMode == Place)
	{

		//place object
		
		OnPhotoTaken.Broadcast();

		//get the slot that player currently has selected
		FStorageSlot Slot = StorageSlots[CurrentSlotIndex];

		//check if it can be placed
		if(!Slot.StoredObject->CanPlace())
		{
			
			return;
		}

		//remove it from the camera
		StorageSlots.RemoveAt(CurrentSlotIndex);

		//actually place it inm the world
		Slot.StoredObject->PlaceObjectInWorld();

		//broadcast that placing just happened
		OnObjectPlaced.Broadcast(Slot);

		OnNewCameraMode.Broadcast(CaptureMode);

		//placing always switches us to capture mode
		CurrentCameraMode = CaptureMode;
	}
}


void UCameraControllerComponentV2::CancelCameraUse()
{
	//leave this as is so that swatchlet attack can cancel camera use
	if (CurrentCameraMode == CaptureMode)
	{
		CancelCapture();
	}
}

void UCameraControllerComponentV2::GoBack()
{
	//always go back to capture mode but tear down differently depending on mode
	switch (CurrentCameraMode)
	{
	case Library:
		OnNewCameraMode.Broadcast(CaptureMode);
		CurrentCameraMode = CaptureMode;
		break;
	case Place:
		OnNewCameraMode.Broadcast(CaptureMode);
		CurrentCameraMode = CaptureMode;
		if (StorageSlots.Num() <= 0) return;
		//if currently placing an object make sure to hide object when going back to another mode
		FStorageSlot SelectedSlot = StorageSlots[CurrentSlotIndex];
		SelectedSlot.StoredObject->SetPreviewVisibilityPlacementOfObject(false);
		SelectedSlot.StoredObject->CancelPlace();
		break;
	}
}

void UCameraControllerComponentV2::GoToLibrary()
{
	//make sure you can actualyl go to library
	if(CurrentCameraMode != CaptureMode) return;
	if(bIsCameraChargingUp) return;

	//broadcast that we switched to library
	OnNewCameraMode.Broadcast(Library);
	CurrentCameraMode = Library;

	//switch to the most recent stored object, or none if there's nothing in the camera
	if (StorageSlots.Num() <= 0)
	{
		SwitchToSlot(-1);
	}
	else
	{
		SwitchToSlot(StorageSlots.Num() - 1);
	}
}

void UCameraControllerComponentV2::StartCapture()
{
	//make sure there's something focusable to capture
	if (!FocusedStorable)
	{
		OnNewCaptureMessage.Broadcast(NoFocusObject);
		return;
	}

	//make sure camera has enough space
	if (StorageSlots.Num() >= CameraObjectCapacity)
	{
		OnNewCaptureMessage.Broadcast(NoMoreSpace);
		return;
	}

	//start capture
	OnPhotoTaken.Broadcast();
	OnCaptureStatusUpdate.Broadcast(Charging);

	//setup state information
	bIsCameraChargingUp = true;
	NumberOfCaptureSegmentsCompleted = 0;

	//setup timer
	GetWorld()->GetTimerManager().SetTimer(CaptureHoldTimer, this,
	                                       &UCameraControllerComponentV2::CheckCurrentCaptureSegment,
	                                       FocusedStorable->TimeOfACaptureSegment, false);
	bCameraReadyToEndSuccessfulCaptureSequence = false;
}

void UCameraControllerComponentV2::EndCaptureCharge()
{
	//make sure that camera is in correct state to end capture
	if(!bIsCameraChargingUp) return;

	
	if (bCameraReadyToEndSuccessfulCaptureSequence) //check if camera was ready to end a capture
	{
		EndCaptureSuccessfully();
	}
	else
	{
		//camera failed because of some reason so mark capture has cancelled even if fully charged
		OnNewCaptureMessage.Broadcast(Cancelled);
		CancelCapture();
	}
}

void UCameraControllerComponentV2::CancelCapture()
{
	//reset camera information
	NumberOfCaptureSegmentsCompleted = 0;
	bIsCameraChargingUp = false;
	if (CaptureHoldTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CaptureHoldTimer);
	}
	OnCaptureStatusUpdate.Broadcast(Canceled);
}

void UCameraControllerComponentV2::EndCaptureSuccessfully()
{
	TEnumAsByte<ECaptureMessage> Message;
	if (!IsFocusedStorableStillInFrame(Message)) //last check to make sure the object is focused in frame
	{
		//Ended up failing last check so mark capture as cancelled which is same as failure
		OnNewCaptureMessage.Broadcast(Message);
		bIsCameraChargingUp = false;
		CancelCapture();
		return;
	}

	//broadcast events and reset state info
	bIsCameraChargingUp = false;
	OnNewCaptureMessage.Broadcast(Captured);
	OnCaptureStatusUpdate.Broadcast(Completed);
	NumberOfCaptureSegmentsCompleted = 0;

	//actually store the captured objects inside of the camera
	StoreObject(FocusedStorable);
}

bool UCameraControllerComponentV2::IsFocusedStorableStillInFrame(TEnumAsByte<ECaptureMessage>& CaptureError)
{

	//just a ton of checks to make sure the focused object can actually be stored
	bool bCanBeStored = FocusedStorable->CanBeStoredRightNow();
	if(!bCanBeStored)
	{
		CaptureError = InsideOfObject;
	}
	
	bool bIsStillInFrame = CaptureValidator->ValidateStorableInFrame();
	if (!bIsStillInFrame)
	{
		CaptureError = LowVisibility;
	}


	float Distance = FVector::Distance(FocusedStorable->GetOwner()->GetActorLocation(),
	                                   ActualRecordingCameraComponent->GetComponentLocation());
	bool bIsWithinDistance = Distance <= MaxDistanceForCapturing;
	if (!bIsWithinDistance)
	{
		CaptureError = OutsideOfDistance;
	}

	return bIsStillInFrame && bIsWithinDistance && bCanBeStored;
}

void UCameraControllerComponentV2::CheckCurrentCaptureSegment()
{
	//run all check logic
	TEnumAsByte<ECaptureMessage> Error;
	if (!IsFocusedStorableStillInFrame(Error))
	{
		NumberOfCaptureSegmentsCompleted = 0;
		MarkCaptureAsFailed(Error);
		return;
	}

	//mark the segment as completed
	NumberOfCaptureSegmentsCompleted++;
	OnACaptureSegmentCompleted.Broadcast(NumberOfCaptureSegmentsCompleted);
	if (NumberOfCaptureSegmentsCompleted >= FocusedStorable->NumberOfCaptureSegments)
	{
		MarkCaptureAsReadyToFinish();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(CaptureHoldTimer, this,
	                                       &UCameraControllerComponentV2::CheckCurrentCaptureSegment,
	                                       FocusedStorable->TimeOfACaptureSegment, false);
}

void UCameraControllerComponentV2::MarkCaptureAsFailed(TEnumAsByte<ECaptureMessage> Error)
{
	OnNewCaptureMessage.Broadcast(Error);
	bIsCameraChargingUp = false;
	NumberOfCaptureSegmentsCompleted = 0;
	CancelCapture();
}

void UCameraControllerComponentV2::MarkCaptureAsReadyToFinish()
{
	bCameraReadyToEndSuccessfulCaptureSequence = true;
	OnCaptureStatusUpdate.Broadcast(Charged);
}

void UCameraControllerComponentV2::StoreObject(UStorableComponent* Storable)
{
	//setup object to be stored
	Storable->StoreObjectToCamera();

	//create slot to store object
	FStorageSlot Slot;
	Slot.StoredObject = Storable;
	StorageSlots.Add(Slot);
	OnObjectStored.Broadcast(Slot);
	if(Storable->GetOwner()->IsA(ASwatchlet::StaticClass()))
	{
		OnSwatchletCaptured.Broadcast();
	}
}

void UCameraControllerComponentV2::CaptureTick(float DeltaTime)
{
	if (bIsCameraChargingUp) return;
	
	TArray<AActor*> OutActors;

	FVector SphereOrigin = ActualRecordingCameraComponent->GetComponentLocation();
	TArray<AActor*> ActorsToIgnore;


	//only check if objects can be captured by camera if they are within max distance... we just do a simple overlap check
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), SphereOrigin, MaxDistanceForCapturing,
	                                          ObjectTypesOfStorableActors, AActor::StaticClass(), ActorsToIgnore,
	                                          OutActors);

	OutActors.RemoveAll([](AActor* Actor) { return !Actor->GetComponentByClass<UStorableComponent>(); });

	TArray<UStorableComponent*> Storables;
	for (AActor* Actor : OutActors)
	{
		Storables.Add(Actor->GetComponentByClass<UStorableComponent>());
	}

	//run cheap dot product check to do a pseudo check if a storable is in frame
	//also find which storable is closest to center LOS
	AActor* BestActor = nullptr;
	float ClosestDotProduct = 0;
	for (UStorableComponent* StorableComponent : Storables)
	{
		if(!StorableComponent->CanBeStoredRightNow()) continue;
		if (!IsTargetWithinFrame(StorableComponent->GetOwner()->GetRootComponent())) continue;

		//mark storables as valid for things like VFX and whatnot
		int StartArraySize = LastFrameValidStorables.Num();
		LastFrameValidStorables.Remove(StorableComponent);
		if (StartArraySize == LastFrameValidStorables.Num())
		{
			StorableComponent->ChangeWorldStorableState(Valid);
		}

		//compare with current best target
		float DotProduct = GetDotProductForTarget(StorableComponent->GetOwner()->GetRootComponent());
		if (DotProduct > ClosestDotProduct)
		{
			BestActor = StorableComponent->GetOwner();
			ClosestDotProduct = DotProduct;
		}
	}

	//mark as invalid
	for (UStorableComponent* NonValidStorables : LastFrameValidStorables)
	{
		NonValidStorables->ChangeWorldStorableState(Invalid);
	}

	LastFrameValidStorables = Storables;

	//setup state for best target
	if (BestActor)
	{
		if (FocusedStorable == BestActor->GetComponentByClass<UStorableComponent>()) return;
		if (FocusedStorable) FocusedStorable->ChangeWorldStorableState(Valid);
		FocusedStorable = BestActor->GetComponentByClass<UStorableComponent>();
		FocusedStorable->ChangeWorldStorableState(Focused);
		return;
	}

	
	if (FocusedStorable == nullptr) return;
	FocusedStorable->ChangeWorldStorableState(Valid);
	FocusedStorable = nullptr;
}

void UCameraControllerComponentV2::ScrollUpInObjectStorage()
{
	if (StorageSlots.Num() <= 0)
	{
		CurrentSlotIndex = 0;
		return;
	}

	int32 TempStorageSlotIndex = CurrentSlotIndex;
	TempStorageSlotIndex--;
	if (TempStorageSlotIndex < 0) //loop back to end
	{
		TempStorageSlotIndex = StorageSlots.Num() - 1;
	}
	//actually switch to that slot
	SwitchToSlot(TempStorageSlotIndex);
}

void UCameraControllerComponentV2::ScrollDownInObjectStorage()
{
	if (StorageSlots.Num() <= 0)
	{
		CurrentSlotIndex = 0;
		return;
	}

	int32 TempStorageSlotIndex = CurrentSlotIndex;
	TempStorageSlotIndex++;
	if (TempStorageSlotIndex >= StorageSlots.Num()) //loop back to start
	{
		TempStorageSlotIndex = 0;
	}
	//actually switch to that slot
	SwitchToSlot(TempStorageSlotIndex);
}

void UCameraControllerComponentV2::SwitchToSlot(int32 Index)
{
	CurrentSlotIndex = Index;
	OnSelectionChanged.Broadcast(CurrentSlotIndex);
}

void UCameraControllerComponentV2::SelectPhotoForPlacing()
{
	if(CurrentCameraMode != Library) return;
	if (StorageSlots.Num() <= 0) return;
	//setup state for placing
	FStorageSlot SelectedSlot = StorageSlots[CurrentSlotIndex];
	if(SelectedSlot.StoredObject->GetOwner()->IsA(ASwatchlet::StaticClass()))
	{
		OnLibraryMessage.Broadcast(CantPlace);
		return;
	}
	SelectedSlot.StoredObject->SetPreviewVisibilityPlacementOfObject(true);
	SelectedSlot.StoredObject->StartPlace();
	CurrentCameraMode = Place;
	//broadcast neccesary events
	OnLibraryMessage.Broadcast(SuccessfulySelected);
	OnNewCameraMode.Broadcast(Place);
}

void UCameraControllerComponentV2::HandlePlacingTick(float DeltaTime)
{
	//make sure slot index is valid 
	if (CurrentSlotIndex < 0 || CurrentSlotIndex >= StorageSlots.Num()) return;

	//get the slot that has been selected to place
	FStorageSlot Container = StorageSlots[CurrentSlotIndex];

	//rotation logic
	bool bDoRotationLogic = false;
	float RotationMultiplier = 0;
	if(bIsRotatingObjectRight && !bIsRotatingObjectLeft)
	{

		bDoRotationLogic = true;
		RotationMultiplier = 1;
	} else if(bIsRotatingObjectLeft && !bIsRotatingObjectRight)
	{

		bDoRotationLogic = true;
		RotationMultiplier = -1;
	}

	if(bDoRotationLogic)
	{
		Container.StoredObject->GetOwner()->AddActorLocalRotation(FRotator(0,RotationMultiplier * ObjectRotationSpeed * DeltaTime,0));
	}

	//calculate where player wants to place as well as validity of placement

	FHitResult HitResult;

	//get size and position of box trace that will be used to calculate placement
	FVector BoxExtents = Container.StoredObject->GetHalfBoundsForPlacing();
	FVector Offset = Container.StoredObject->GetBoundsOffset();

	//where should start and end of the traces be
	FVector StartTraceLocation = ActualRecordingCameraComponent->GetComponentLocation();
	FVector EndTraceLocation = StartTraceLocation + ActualRecordingCameraComponent->GetForwardVector() *
		MaxDistanceForCapturing;

	FRotator Rotation = Container.StoredObject->GetOwner()->GetActorRotation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	ActorsToIgnore.Add(Container.StoredObject->GetOwner());


	//multi box trace to detect all areas that a player could be targeting, and more importantly will be used to detect the segments that the box didn't collide
	TArray<FHitResult> HitResults;
	bool bDidMultiHit = UKismetSystemLibrary::BoxTraceMulti(GetWorld(), StartTraceLocation, EndTraceLocation,
	                                                        BoxExtents,
	                                                        Rotation, TraceTypeForFrameDetection, false, ActorsToIgnore,
	                                                        EDrawDebugTrace::Type::ForOneFrame,
	                                                        HitResults, true);

	//if the multi box trace hit nothing at all just place at the end of the multi box trace
	if (HitResults.Num() <= 0)
	{
		Container.StoredObject->UpdatePreviewPlacement(EndTraceLocation - Offset, Container.StoredObject->GetOwner()->GetActorRotation(), true);
		PlaceMessage = ValidPlacement;
		return;
	}

	bool bFoundValidPlacement = false;
	FHitResult BestHitResult;


	//loop backwards through the results of the multi box trace
	//basically what it does is it checks if the location is big enough for the object
	//then it checks if the location is visible to the player
	for (int i = HitResults.Num() - 1; i >= 0; i--)
	{
		FHitResult Hit = HitResults[i];


		TArray<AActor*> OverlapIgnoreActors;
		TArray<AActor*> OutActors;
		OverlapIgnoreActors.Add(Container.StoredObject->GetOwner());
		//check if the hit location allows for non box overlaps when placing above location
		UKismetSystemLibrary::BoxOverlapActors(GetWorld(), Hit.Location, BoxExtents - FVector(10), //10cm is for hardcoded leniency
		                                       ObjectTypesForPlaceOverlap, AActor::StaticClass(), OverlapIgnoreActors,
		                                       OutActors);

		if (OutActors.Num() > 0)
		{
			continue;
		}

		//do a line trace to confirm visiblity of this location
		FHitResult LineTraceHit;
		bool bLineTraceHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), StartTraceLocation, Hit.Location,
		                                                           VisibilityTraceType, false, ActorsToIgnore,
		                                                           EDrawDebugTrace::Type::None, LineTraceHit,
		                                                           true);

		//run logic if it was valid
		if (!bLineTraceHit)
		{
			bFoundValidPlacement = true;
			BestHitResult = Hit;
			break;
		}

		if (Hit.Component == LineTraceHit.Component)
		{
			bFoundValidPlacement = true;
			BestHitResult = Hit;
			break;
		}
	}

	//no valid positions at all so mark object in an invalid positoin
	if (!bFoundValidPlacement)
	{
		Container.StoredObject->UpdatePreviewPlacement(HitResults[0].Location - Offset, Container.StoredObject->GetOwner()->GetActorRotation(), false);
		PlaceMessage = InvalidPlacement;
		return;
	}

	//mark object in a valid placement to placed when player wants
	PlaceMessage = ValidPlacement;
	Container.StoredObject->UpdatePreviewPlacement(BestHitResult.Location - Offset, Container.StoredObject->GetOwner()->GetActorRotation(), true);
	
}

void UCameraControllerComponentV2::PlaceModeInit()
{
	if (StorageSlots.Num() <= 0) return;
	FStorageSlot SelectedSlot = StorageSlots[CurrentSlotIndex];
	SelectedSlot.StoredObject->SetPreviewVisibilityPlacementOfObject(true);
}

void UCameraControllerComponentV2::PlaceModeTeardown()
{
	if (StorageSlots.Num() <= 0) return;
	FStorageSlot SelectedSlot = StorageSlots[CurrentSlotIndex];
	SelectedSlot.StoredObject->SetPreviewVisibilityPlacementOfObject(false);
}

float UCameraControllerComponentV2::GetDotProductForTarget(USceneComponent* TargetComponent) const
{
	//dot product calculations
	FVector Heading = TargetComponent->GetComponentLocation() - ActualRecordingCameraComponent->GetComponentLocation();
	Heading = Heading.GetSafeNormal();
	float FacingDot = FVector::DotProduct(ActualRecordingCameraComponent->GetForwardVector(), Heading);
	return FacingDot;
}

bool UCameraControllerComponentV2::IsTargetWithinFrame(USceneComponent* TargetComponent) const
{
	if (!TargetComponent) return false;
	//check if is within max distance
	float Distance = FVector::Distance(ActualRecordingCameraComponent->GetComponentLocation(),
	                                   TargetComponent->GetComponentLocation());
	if (Distance > MaxDistanceForCapturing) return false;

	//check if is within cone of mock view(los check basically)
	float Dot = GetDotProductForTarget(TargetComponent);
	if (Dot < ConeDot) return false;

	FHitResult Hit;

	//do a simple line trace to center of object for simple visiblity check
	FVector TraceStart = ActualRecordingCameraComponent->GetComponentLocation();
	FVector TraceEnd = TargetComponent->GetComponentLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(TargetComponent->GetOwner());


	ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceTypeForFrameDetection);

	bool bDidHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, CollisionChannel, QueryParams);
	if (bDidHit) return false;

	return true;
}
