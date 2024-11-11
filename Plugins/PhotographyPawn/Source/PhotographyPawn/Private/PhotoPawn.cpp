// Fill out your copyright notice in the Description page of Project Settings.


#include "PhotoPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Serialization/MemoryWriter.h"
#include "ImageUtils.h"

// Sets default values
APhotoPawn::APhotoPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	//Scene capture component will copy the camera and render what it sees to render texture
	//render texture will be saved as a .png for later use
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Scene Capture"));
	SceneCapture->SetupAttachment(Camera);
	
}

// Called when the game starts or when spawned
void APhotoPawn::BeginPlay()
{
	Super::BeginPlay();

	Camera->PostProcessSettings.bOverride_DepthOfFieldFstop = 1;
	LensFocalLengthMM = CameraLens.MinFocalLengthMM;
	ApertureIndex = 0;
}

float APhotoPawn::FocalLengthtoFOV(float FocalLength)
{
	return FMath::RadiansToDegrees(2 * FMath::Atan(35.f/(2*FocalLength)));
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

}

void APhotoPawn::CapturePhoto()
{
	//capture camera view to render target
	SceneCapture->UpdateContent();
	
	if(UTextureRenderTarget2D* RenderTarget = SceneCapture->TextureTarget)
	{
		FString SavePath = FPaths::ProjectSavedDir() / TEXT("TestSave.png");
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
		}
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
}

void APhotoPawn::ChangeAperture(float ApertureDelta)
{
	ApertureIndex = FMath::Clamp(ApertureIndex + (1*ApertureDelta), 0, CameraLens.FStops.Num());
	
	float Aperture = CameraLens.FStops[ApertureIndex];
	Camera->PostProcessSettings.DepthOfFieldFstop = Aperture;
}

