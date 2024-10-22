// Fill out your copyright notice in the Description page of Project Settings.


#include "ScreenCaptureCharacter.h"

#include "EnhancedInputComponent.h"
#include "Components/CameraControllerComponent.h"

#include "Components/CapsuleComponent.h"

#include "Components/ScreenCaptureCharacterMovementComponent.h"

#include "ScreenCapture/Items/Components/InteractableComponent.h"
#include "ScreenCapture/UI/ScreenCaptureGameUserSettings.h"

FName AScreenCaptureCharacter::FirstPersonCameraComponentName(TEXT("FirstPersonCamera"));
FName AScreenCaptureCharacter::CameraControllerComponentName(TEXT("CameraController"));
FName AScreenCaptureCharacter::SceneCaptureComponentName(TEXT("SceneCapture2D"));
FName AScreenCaptureCharacter::CaptureValidatorComponentName(TEXT("CaptureValidator"));
FName AScreenCaptureCharacter::ArmsMeshComponentName(TEXT("ArmsMesh"));
FName AScreenCaptureCharacter::CameraMeshComponentName(TEXT("CameraMesh"));
FName AScreenCaptureCharacter::CameraScreenComponentName(TEXT("CameraScreen"));
FName AScreenCaptureCharacter::CameraUIComponentName(TEXT("CameraUI"));
FName AScreenCaptureCharacter::FootstepControllerComponentName(TEXT("FootstepSounds"));
FName AScreenCaptureCharacter::DeathControllerComponentName(TEXT("DeathController"));


// Sets default values
AScreenCaptureCharacter::AScreenCaptureCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UScreenCaptureCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ScreenCaptureCharacterMovement = Cast<UScreenCaptureCharacterMovementComponent>(GetCharacterMovement());

	InteractDistance = 450;

	CameraControllerV2Component = CreateDefaultSubobject<UCameraControllerComponentV2>(CameraControllerComponentName);

	DeathControllerComponent = CreateDefaultSubobject<UPlayerDeathController>(DeathControllerComponentName);

	StaggerFallSpeed = 600;

	LandSlowDownSpeedWhileRunning = 150;
	LandSlowDownSpeedWhileWalking = 75;


	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(
		AScreenCaptureCharacter::FirstPersonCameraComponentName);
	if (FirstPersonCamera)
	{
		FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
		FirstPersonCamera->SetRelativeLocation(FVector(-10, 0, 60));

		ArmsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(
			AScreenCaptureCharacter::ArmsMeshComponentName);
		if (ArmsMeshComponent)
		{
			ArmsMeshComponent->SetupAttachment(FirstPersonCamera);
			ArmsMeshComponent->SetRelativeLocation(FVector(-30, 0, -150));
		}

		CameraSMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(
			AScreenCaptureCharacter::CameraMeshComponentName);
		if (CameraSMeshComponent)
		{
			CameraSMeshComponent->SetupAttachment(FirstPersonCamera);
			CameraSMeshComponent->SetRelativeLocation(FVector(23, 0, 0));
			CameraSMeshComponent->SetRelativeRotation(FRotator(0, 0, -90));
			CameraSMeshComponent->SetRelativeScale3D(FVector(0.0725));
			if (CameraControllerV2Component)
			{
				CameraControllerV2Component->SetMeshComponent(CameraSMeshComponent);
			}

			// SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(
			// 	AScreenCaptureCharacter::SceneCaptureComponentName);
			// if (SceneCaptureComponent)
			// {
			// 	SceneCaptureComponent->SetupAttachment(CameraSMeshComponent);
			// 	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
			// 	SceneCaptureComponent->SetRelativeLocation(FVector(0, 200, 0));
			// 	SceneCaptureComponent->SetRelativeRotation(FRotator(0, 0, 90));
			// 	if (CameraControllerComponent)
			// 	{
			// 		CameraControllerComponent->SetRecordingCameraComponent(SceneCaptureComponent);
			// 	}
			// }

			// CaptureModeValidatorComponent = CreateDefaultSubobject<UCaptureModeValidator>(CaptureValidatorComponentName);
			// if(CaptureModeValidatorComponent)
			// {
			// 	CaptureModeValidatorComponent->SetupAttachment(CameraSMeshComponent);
			// 	SceneCaptureComponent->SetRelativeLocation(FVector(0, 200, 0));
			// 	SceneCaptureComponent->SetRelativeRotation(FRotator(0, 0, 90));
			// 	if(CameraControllerComponent)
			// 	{
			// 		CameraControllerComponent->SetCaptureValidator(CaptureModeValidatorComponent);
			// 	}
			// }

			// CameraScreenComponent = CreateDefaultSubobject<UCameraScreen>(
			// 	AScreenCaptureCharacter::CameraScreenComponentName);
			// if (CameraScreenComponent)
			// {
			// 	CameraScreenComponent->SetupAttachment(CameraSMeshComponent,"Camera_v5_Joint_CameraBody");
			// 	CameraScreenComponent->SetRelativeLocation(FVector(0, -60, -10));
			// 	CameraScreenComponent->SetRelativeRotation(FRotator(90, 0, 180));
			// 	CameraScreenComponent->SetRelativeScale3D(FVector(2.375, 1.4, 2.375));
			//
			// 	if(SceneCaptureComponent) CameraScreenComponent->SetSceneCapture(SceneCaptureComponent);
			// 	if(CaptureModeValidatorComponent) CameraScreenComponent->SetCaptureValidator(CaptureModeValidatorComponent);
			//
			// 	CameraUIDisplayComponent = CreateDefaultSubobject<UWidgetComponent>(
			// 		AScreenCaptureCharacter::CameraUIComponentName);
			// 	if (CameraUIDisplayComponent)
			// 	{
			// 		CameraUIDisplayComponent->SetupAttachment(CameraScreenComponent);
			// 		CameraUIDisplayComponent->SetRelativeRotation(FRotator(-180, 90, -90));
			// 		CameraUIDisplayComponent->SetDrawSize(FVector2D(1920, 1080));
			// 	}
			// }
		}
	}
}

void AScreenCaptureCharacter::SetCurrentPlayerCameraManagerLocation(FVector Location)
{
	CurrentPlayerCameraManagerLocation = Location;
	CameraSMeshComponent->SetWorldLocation(Location + FirstPersonCamera->GetForwardVector() * CameraOffset.Length());
}

void AScreenCaptureCharacter::FinishedLanding()
{
	bAbleToJump = true;
	ScreenCaptureCharacterMovement->MaxWalkSpeed = ScreenCaptureCharacterMovement->bIsRunning
		                                               ? ScreenCaptureCharacterMovement->RunSpeed
		                                               : ScreenCaptureCharacterMovement->StoredWalkSpeed;
}

void AScreenCaptureCharacter::Jump()
{
	if (bAbleToJump && !bIsCrouched)
	{
		Super::Jump();
		bAbleToJump = false;
	}
}


void AScreenCaptureCharacter::LandedCallback(const FHitResult& Result)
{
	FVector LandVelocity = GetVelocity();
	ScreenCaptureCharacterMovement->MaxWalkSpeed = ScreenCaptureCharacterMovement->bIsRunning
		                                               ? LandSlowDownSpeedWhileRunning
		                                               : LandSlowDownSpeedWhileWalking;
	OnLanded.Broadcast(LandVelocity);
	if (FMath::Abs(LandVelocity.Z) > StaggerFallSpeed)
	{
		ScreenCaptureCharacterMovement->DisableMovement();
	}
}

// Called when the game starts or when spawned
void AScreenCaptureCharacter::BeginPlay()
{
	Super::BeginPlay();

	Settings = Cast<UScreenCaptureGameUserSettings>(UGameUserSettings::GetGameUserSettings());

	bAbleToJump = true;

	CameraOffset = CameraSMeshComponent->GetComponentLocation() - FirstPersonCamera->GetComponentLocation();

	FDetachmentTransformRules DetachRules = FDetachmentTransformRules(EDetachmentRule::KeepRelative, false);
	CameraSMeshComponent->DetachFromComponent(DetachRules);

	FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,
	                                                            EAttachmentRule::KeepWorld, false);
	CameraSMeshComponent->AttachToComponent(
		Cast<APlayerController>(Controller)->PlayerCameraManager->GetRootComponent(),
		Rules);

	StartLocation = GetActorLocation();

	LandedDelegate.AddDynamic(this, &AScreenCaptureCharacter::LandedCallback);

	// CameraControllerComponent->OnCameraTogglePower.AddDynamic(CameraScreenComponent, &UCameraScreen::TurnScreenOnOrOff);

	float CameraZ = FirstPersonCamera->GetRelativeLocation().Z;
	float CameraUnscaledHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	CapsuleCameraOffset = CameraUnscaledHeight - CameraZ;

	AddTickPrerequisiteComponent(ScreenCaptureCharacterMovement);
}

void AScreenCaptureCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	AddMovementInput(GetActorRightVector(), MovementVector.X);
	AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	if (MovementVector.Y <= 0.2f)
	{
		StopRunning();
	}
}

void AScreenCaptureCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	if (Settings)
	{
		LookVector *= Settings->GetSensitivity();
	}
	AddControllerYawInput(LookVector.X * 0.5f);
	AddControllerPitchInput(LookVector.Y * 0.5f);
}

void AScreenCaptureCharacter::ToggleCameraState()
{
	// if(CameraControllerComponent->)
	if (CameraControllerV2Component->CurrentCameraState == Stowed)
	{
		CameraControllerV2Component->SwitchCameraState(PulledOut);
	}
	else
	{
		CameraControllerV2Component->SwitchCameraState(Stowed);
	}
}

void AScreenCaptureCharacter::FinishedHoldOnTakePhoto()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;
	CameraControllerV2Component->EndUseCamera();
}

void AScreenCaptureCharacter::StartedTakePhoto()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;
	CameraControllerV2Component->StartUseCamera();
}

void AScreenCaptureCharacter::CanceledTakePhoto()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;
	// CameraControllerV2Component->CancelCameraUse(); //TODO... probably don't need this stuff at all
}

void AScreenCaptureCharacter::SwitchCameraModeLeft()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;

	// if (CameraControllerV2Component->GetCurrentCameraState() == Stowed) return;
	// int32 CameraMode = CameraControllerV2Component->GetCurrentCameraMode();
	// CameraMode--;
	// if (CameraMode < 0)
	// {
	// 	CameraMode = 1;
	// }
	// CameraControllerV2Component->SwitchCameraMode(TEnumAsByte<ECameraMode>(CameraMode));
}

void AScreenCaptureCharacter::SwitchCameraModeRight()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;

	// if (CameraControllerV2Component->GetCurrentCameraState() == Stowed) return;
	// int32 CameraMode = CameraControllerV2Component->GetCurrentCameraMode();
	// CameraMode++;
	// if (CameraMode >= 2)
	// {
	// 	CameraMode = 0;
	// }
	// CameraControllerV2Component->SwitchCameraMode(TEnumAsByte<ECameraMode>(CameraMode));
}

void AScreenCaptureCharacter::GoBack()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;

	CameraControllerV2Component->GoBack();
}

void AScreenCaptureCharacter::GoToLibrary()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;

	CameraControllerV2Component->GoToLibrary();
}

void AScreenCaptureCharacter::SelectPhotoForPlacing()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;

	CameraControllerV2Component->SelectPhotoForPlacing();
}

void AScreenCaptureCharacter::ScrollUp()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;

	CameraControllerV2Component->ScrollUpInObjectStorage();
}

void AScreenCaptureCharacter::ScrollDown()
{
	if(CameraControllerV2Component->CurrentCameraState == Stowed) return;

	CameraControllerV2Component->ScrollDownInObjectStorage();
}

void AScreenCaptureCharacter::ToggleRun()
{
	// if(GetWorld()->GetTimerManager().IsTimerActive(FootstepTimerHandle))
	// {
	// 	GetWorld()->GetTimerManager().ClearTimer(FootstepTimerHandle);
	// }
	ScreenCaptureCharacterMovement->bWantsToRun = !ScreenCaptureCharacterMovement->bIsRunning;
	// if(ScreenCaptureCharacterMovement->bIsRunning)
	// {
	// 	GetWorld()->GetTimerManager().SetTimer(FootstepTimerHandle, this, &AScreenCaptureCharacter::Footstep, DurationBetweenRunningFootsteps, true);
	// } else
	// {
	// 	GetWorld()->GetTimerManager().SetTimer(FootstepTimerHandle, this, &AScreenCaptureCharacter::Footstep, DurationBetweenWalkingFootsteps, true);
	// }
}

void AScreenCaptureCharacter::StartRunning()
{
	if (ScreenCaptureCharacterMovement)
	{
		if (CameraControllerV2Component->CurrentCameraState == PulledOut) return;
		ScreenCaptureCharacterMovement->bWantsToRun = true;
		// ScreenCaptureCharacterMovement->bWantsToCrouch = false;
	}
}

void AScreenCaptureCharacter::StopRunning()
{
	// GetWorld()->GetTimerManager().ClearTimer(FootstepTimerHandle);
	// if(GetLastMovementInputVector().Length() > 0.1f)
	// {
	// 	GetWorld()->GetTimerManager().SetTimer(FootstepTimerHandle, this, &AScreenCaptureCharacter::Footstep, DurationBetweenWalkingFootsteps, true);
	// }
	ScreenCaptureCharacterMovement->bWantsToRun = false;
}

void AScreenCaptureCharacter::RecalculateCameraHeightLevel()
{
	//todo make this better so we aren't forced to run this every frame
	float CameraZ = FirstPersonCamera->GetRelativeLocation().Z;
	float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float Offset = CapsuleHalfHeight - CameraZ;
	if (FMath::Abs(Offset - CapsuleCameraOffset) < 1) return;
	FVector RelativeLocation = FirstPersonCamera->GetRelativeLocation();
	RelativeLocation.Z = CapsuleHalfHeight - CapsuleCameraOffset;
	FirstPersonCamera->SetRelativeLocation(RelativeLocation);
}

void AScreenCaptureCharacter::Interact()
{
	if (Interactable)
	{
		Interactable->HandleInteract();
	}
}


void AScreenCaptureCharacter::CheckForInteract()
{
	FHitResult HitResult;
	ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceTypesForInteractables);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, FirstPersonCamera->GetComponentLocation(),
	                                                 FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->
	                                                 GetForwardVector() * InteractDistance, CollisionChannel, Params);
	if (!bHit)
	{
		Interactable = nullptr;
		return;
	}
	if (UInteractableComponent* Component = HitResult.GetActor()->GetComponentByClass<UInteractableComponent>())
	{
		Interactable = Component;
	}
	else
	{
		Interactable = nullptr;
	}
}

void AScreenCaptureCharacter::Crouch(bool bClientSimulation)
{
	if (ScreenCaptureCharacterMovement)
	{
		if (CanCrouch())
		{
			ScreenCaptureCharacterMovement->bWantsToCrouch = true;
			// ScreenCaptureCharacterMovement->bWantsToRun = false;
		}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		else if (!GetCharacterMovement()->CanEverCrouch())
		{
			UE_LOG(LogTemp, Log,
			       TEXT(
				       "%s is trying to crouch, but crouching is disabled on this character! (check CharacterMovement NavAgentSettings)"
			       ), *GetName());
		}
#endif
	}
}


// Called every frame
void AScreenCaptureCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckForInteract();
	RecalculateCameraHeightLevel();
}

// Called to bind functionality to input
void AScreenCaptureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent) return;

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AScreenCaptureCharacter::Move);

	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AScreenCaptureCharacter::Look);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AScreenCaptureCharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
	                                   &AScreenCaptureCharacter::StopJumping);

	// EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AScreenCaptureCharacter::ToggleRun);
	// EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this,
	//                                    &AScreenCaptureCharacter::StopRunning);

	// EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this,
	//                                    &AScreenCaptureCharacter::StartRunning);
	// EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this,
	//                                    &AScreenCaptureCharacter::StopRunning);
	// EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this,
	//                                    &AScreenCaptureCharacter::StopRunning);

	EnhancedInputComponent->BindAction(TogglePullOutCamera, ETriggerEvent::Triggered, this,
	                                   &AScreenCaptureCharacter::ToggleCameraState);

	//TODO have each different camera mode be a different IMC
	EnhancedInputComponent->BindAction(TakePhotoAction, ETriggerEvent::Completed, this,
	                                   &AScreenCaptureCharacter::FinishedHoldOnTakePhoto);
	EnhancedInputComponent->BindAction(TakePhotoAction, ETriggerEvent::Started, this,
	                                   &AScreenCaptureCharacter::StartedTakePhoto);

	EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Started, this,
	                                   &AScreenCaptureCharacter::GoBack);

	EnhancedInputComponent->BindAction(GoToLibraryAction, ETriggerEvent::Started, this,
	                                   &AScreenCaptureCharacter::GoToLibrary);

	EnhancedInputComponent->BindAction(SwitchCameraModeLeftAction, ETriggerEvent::Triggered, this,
	                                   &AScreenCaptureCharacter::SwitchCameraModeLeft);
	EnhancedInputComponent->BindAction(SwitchCameraModeRightAction, ETriggerEvent::Triggered, this,
	                                   &AScreenCaptureCharacter::SwitchCameraModeRight);

	EnhancedInputComponent->BindAction(ScrollDownInStorageAction, ETriggerEvent::Triggered, this,
	                                   &AScreenCaptureCharacter::ScrollDown);
	EnhancedInputComponent->BindAction(ScrollUpInStorageAction, ETriggerEvent::Triggered, this,
	                                   &AScreenCaptureCharacter::ScrollUp);

	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this,
	                                   &AScreenCaptureCharacter::Interact);

	EnhancedInputComponent->BindAction(SelectForPlacingAction, ETriggerEvent::Triggered, this,
	                                   &AScreenCaptureCharacter::SelectPhotoForPlacing);


	EnhancedInputComponent->BindAction(RotateObjectRightAction, ETriggerEvent::Started, this, &AScreenCaptureCharacter::MarkRotateRight);
	EnhancedInputComponent->BindAction(RotateObjectRightAction, ETriggerEvent::Completed, this,
								   &AScreenCaptureCharacter::UnmarkRotateRight);
	EnhancedInputComponent->BindAction(RotateObjectRightAction, ETriggerEvent::Canceled, this,
							   &AScreenCaptureCharacter::UnmarkRotateRight);

	EnhancedInputComponent->BindAction(RotateObjectLeftAction, ETriggerEvent::Started, this, &AScreenCaptureCharacter::MarkRotateLeft);
	EnhancedInputComponent->BindAction(RotateObjectLeftAction, ETriggerEvent::Completed, this,
								   &AScreenCaptureCharacter::UnmarkRotateLeft);
	EnhancedInputComponent->BindAction(RotateObjectLeftAction, ETriggerEvent::Canceled, this,
							   &AScreenCaptureCharacter::UnmarkRotateLeft);
}
