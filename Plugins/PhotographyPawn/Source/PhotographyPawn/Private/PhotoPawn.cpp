// Fill out your copyright notice in the Description page of Project Settings.


#include "PhotoPawn.h"


#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Serialization/MemoryWriter.h"
#include "ImageUtils.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
APhotoPawn::APhotoPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetRootComponent());

	//Scene capture component will copy the camera and render what it sees to render texture
	//render texture will be saved as a .png for later use
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Scene Capture"));
	SceneCapture->SetupAttachment(Camera);
	
	GetMesh()->SetupAttachment(Camera);

	CameraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CameraMesh"));
	CameraMesh->SetupAttachment(GetMesh(), FName(TEXT("hand_r")));
}

// Called when the game starts or when spawned
void APhotoPawn::BeginPlay()
{
	Super::BeginPlay();

	CacheDefaultCameraSettings();
	
	Camera->PostProcessSettings.bOverride_DepthOfFieldFstop = 1;
	SceneCapture->PostProcessSettings.bOverride_DepthOfFieldFstop = 1;
	LensFocalLengthMM = CameraLens.MinFocalLengthMM;
	ApertureIndex = 0;
	ChangeAperture(0.f);
}

float APhotoPawn::FocalLengthtoFOV(float FocalLength)
{
	return FMath::RadiansToDegrees(2 * FMath::Atan(35.f/(2*FocalLength)));
}

void APhotoPawn::CacheDefaultCameraSettings()
{
	DefaultPostProcessSettings = Camera->PostProcessSettings;
	DefaultCameraFOV = Camera->FieldOfView;
	DefaultCameraConstrainAspectRatio = Camera->bConstrainAspectRatio;
	DefaultCameraAspectRatio = Camera->AspectRatio;
}

void APhotoPawn::ResetCameraSettings()
{
	Camera->PostProcessSettings = DefaultPostProcessSettings;
	Camera->SetFieldOfView(DefaultCameraFOV);
	Camera->SetConstraintAspectRatio(DefaultCameraConstrainAspectRatio);
	Camera->SetAspectRatio(DefaultCameraAspectRatio);
}

// Called every frame
void APhotoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APhotoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Controller))
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if(FirstPersonInputMappingContext)
			{
				
			}
		}
	}
}

void APhotoPawn::CapturePhoto()
{
	//capture camera view to render target
	SceneCapture->UpdateContent();
	
	if(UTextureRenderTarget2D* RenderTarget = SceneCapture->TextureTarget)
	{
		FString FileName = FString::Printf(TEXT("Temp/Temp_%d.png"), NumPhotosTaken);
		FString SavePath = FPaths::ProjectSavedDir() / FileName;
		TArray<uint8> OutData;
		FMemoryWriter MemoryWriter(OutData);
		
		bool bSuccess = FImageUtils::ExportRenderTarget2DAsPNG(RenderTarget, MemoryWriter);
		
		if(bSuccess)
		{
			bool bFileWritten = FFileHelper::SaveArrayToFile(OutData, *SavePath);
			if(bFileWritten)
			{
				UE_LOG(LogTemp, Log, TEXT("Render target saved to: %s"), *SavePath);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to save render target to file %s"), *SavePath);
			}
			NumPhotosTaken += 1;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error! Cannot capture photo. Render target not set on Scene Capture Component."));
	}
	//update scene capture component
	//evaluate photo based on trace from the camera, is it in focus? What is in frame?
}

void APhotoPawn::Zoom(float ZoomDelta)
{
	LensFocalLengthMM = FMath::Clamp(
		LensFocalLengthMM+ZoomDelta,
		CameraLens.MinFocalLengthMM,
		CameraLens.MaxFocalLengthMM);
	
	Camera->SetFieldOfView(FocalLengthtoFOV(LensFocalLengthMM));
	UE_LOG(LogTemp, Log, TEXT("Focal Length: %f"), LensFocalLengthMM);
}

void APhotoPawn::ChangeAperture(float ApertureDelta)
{

	if(CameraLens.FStops.Num() < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Warning! Lens settings on photo pawn contains no FStops. Cannot set aperture."));
		return;
	}
	
	ApertureIndex = FMath::Clamp(ApertureIndex + (1*ApertureDelta), 0, CameraLens.FStops.Num()-1);
	
	FStop = CameraLens.FStops[ApertureIndex];
	Camera->PostProcessSettings.DepthOfFieldFstop = FStop;
	SceneCapture->PostProcessSettings.DepthOfFieldFstop = FStop;
	UE_LOG(LogTemp, Log, TEXT("Aperture: %f"), FStop);
}

void APhotoPawn::AutoFocus()
{

	//draw line trace from camera and set post process focal distance if you hit something
	FVector StartLocation = Camera->GetComponentLocation();
	FVector EndLocation = StartLocation + Camera->GetForwardVector()*100000.f;

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		CollisionParams);

	if(!bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Focus trace failed to hit anything. Focal distance remains unchanged."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Focus trace successful! Focusing on actor: %s at distance %f"), *HitResult.GetActor()->GetName(), HitResult.Distance);
	Camera->PostProcessSettings.DepthOfFieldFocalDistance = HitResult.Distance;
	SceneCapture->PostProcessSettings.DepthOfFieldFocalDistance = HitResult.Distance;
}

void APhotoPawn::EnterPhotoMode()
{
	Camera->SetConstraintAspectRatio(true);
	Camera->SetAspectRatio(1.33f);

	SceneCapture->FOVAngle = Camera->FieldOfView;
	//disable movement
	//Zoom(0.f);
	//ChangeAperture(0.f);
	//Set Camera FOV
	//Set Scene Capture FOV
	//Set Camera post process settings
	
}

void APhotoPawn::ExitPhotoMode()
{
	ResetCameraSettings();
	//re-enable movement
}
