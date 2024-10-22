// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/CameraControllerComponentV2.h"
#include "Components/CaptureModeValidator.h"
#include "Components/PlayerDeathController.h"
#include "Components/PreviewSceneCapture.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "ScreenCaptureCharacter.generated.h"

class UInteractableComponent;
class UCameraScreen;
class UScreenCaptureCharacterMovementComponent;
class UCameraControllerComponent;
struct FInputActionValue;
class UInputAction;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLandedSignature, FVector)

UCLASS()
class SCREENCAPTURE_API AScreenCaptureCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	static FName FirstPersonCameraComponentName;
	
	static FName CameraControllerComponentName;
	static FName SceneCaptureComponentName;

	static FName ArmsMeshComponentName;
	static FName CameraMeshComponentName;
	static FName CameraScreenComponentName;
	static FName CameraUIComponentName;
	static FName FootstepControllerComponentName;
	static FName DeathControllerComponentName;
	static FName CaptureValidatorComponentName;
	// Sets default values for this character's properties
	AScreenCaptureCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


private:
	//components
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UScreenCaptureCharacterMovementComponent> ScreenCaptureCharacterMovement;

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraControllerComponentV2> CameraControllerV2Component;

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlayerDeathController> DeathControllerComponent;

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> ArmsMeshComponent;
	
	UPROPERTY(Category=Camera, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> CameraSMeshComponent;
	


	
	
	
	//input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	//camera specific input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TogglePullOutCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TakePhotoAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CancelAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* GoToLibraryAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchCameraModeLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchCameraModeRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ScrollUpInStorageAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ScrollDownInStorageAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SelectForPlacingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateObjectRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateObjectLeftAction;

	//interact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interact, meta = (AllowPrivateAccess="true"))
	float InteractDistance;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	UInteractableComponent* Interactable;
	
	UPROPERTY(EditDefaultsOnly, Category = Interact, meta = (AllowPrivateAccess="true"))
	TEnumAsByte<ETraceTypeQuery> TraceTypesForInteractables;

	//camera
	UPROPERTY(Transient)
	FVector CameraOffset;
	
	UPROPERTY()
	FVector CurrentPlayerCameraManagerLocation;

private:
	UPROPERTY(Transient)
	float CapsuleCameraOffset; //offset for determine where to place first person camera when crouching.
	

public:
	void SetCurrentPlayerCameraManagerLocation(FVector Location);

	UFUNCTION(BlueprintCallable)
	UInteractableComponent* GetCurrentInteractable() const {return Interactable;}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Landing, meta = (AllowPrivateAccess="true"))
	float LandSlowDownSpeedWhileRunning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Landing, meta = (AllowPrivateAccess="true"))
	float LandSlowDownSpeedWhileWalking;
	
	UPROPERTY(BlueprintReadWrite)
	bool bAbleToJump;
	
	virtual void FinishedLanding();
	virtual void Jump() override;

	FOnLandedSignature OnLanded;

	UFUNCTION()
	void LandedCallback(const FHitResult& Result);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);


	//capture camera mechanics
	
	void ToggleCameraState();

	void FinishedHoldOnTakePhoto();
	void StartedTakePhoto();
	void CanceledTakePhoto();

	void SwitchCameraModeLeft();
	void SwitchCameraModeRight();

	virtual void MarkRotateRight() {CameraControllerV2Component->MarkRotateRight();}
	virtual void UnmarkRotateRight() {CameraControllerV2Component->UnmarkRotateRight();}

	virtual void MarkRotateLeft() {CameraControllerV2Component->MarkRotateLeft();}
	virtual void UnmarkRotateLeft() {CameraControllerV2Component->UnmarkRotateLeft();}

	void GoBack();

	void GoToLibrary();

	void SelectPhotoForPlacing();
	
	void ScrollUp();
	void ScrollDown();

	void ToggleRun();

	FVector StartLocation;
	bool bRan;
	
	//Interact
	void Interact();

	void CheckForInteract();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Footsteps, meta = (AllowPrivateAccess="true"))
	float StaggerFallSpeed;

	
	UPROPERTY(Transient, BlueprintReadOnly)
	class UScreenCaptureGameUserSettings* Settings;

public:
	virtual void Crouch(bool bClientSimulation) override;

	UFUNCTION(BlueprintCallable)
	void StartRunning();

	UFUNCTION(BlueprintCallable)
	void StopRunning();

	void RecalculateCameraHeightLevel();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
