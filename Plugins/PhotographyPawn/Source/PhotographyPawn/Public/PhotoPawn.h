// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/Scene.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PhotoPawn.generated.h"

class UCameraComponent;
class USceneCaptureComponent2D;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FRealCameraLens
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinFocalLengthMM = 24.f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxFocalLengthMM = 70.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<float> FStops;
};

UCLASS()
class PHOTOGRAPHYPAWN_API APhotoPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APhotoPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components")
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components")
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CameraMesh;
	
	/**
	 * These settings will override the camera and scene capture post process settings. This variable will be changed
	 * when the player updates the camera settings in-game.
	 */
	UPROPERTY(EditAnywhere, Category = "Photography")
	FPostProcessSettings PhotoSettings;

	/**
	 * This variable is used to cache the default post process settings of the player camera.
	 * When exiting photo mode, the camera will revert to these settings. 
	 */
	UPROPERTY(BlueprintReadOnly)
	FPostProcessSettings DefaultPostProcessSettings;

	UPROPERTY(EditAnywhere, Category = "Photography")
	FRealCameraLens CameraLens;

	UPROPERTY(EditAnywhere, Category = "Photography")
	float LensFocalLengthMM;

	float DefaultCameraFOV;
	bool DefaultCameraConstrainAspectRatio;
	float DefaultCameraAspectRatio;
	
	UPROPERTY(EditAnywhere, Category = "Photography")
	int ApertureIndex;

	/**
	 * Number of photos taken in this session of the game. Does not count photos taken in previous playthroughs. 
	 */
	UPROPERTY(BlueprintReadOnly)
	int NumPhotosTaken;

	UFUNCTION()
	float FocalLengthtoFOV(float FocalLength);

	UFUNCTION()
	void CacheDefaultCameraSettings();

	UFUNCTION()
	void ResetCameraSettings();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void CapturePhoto();

	UFUNCTION(BlueprintCallable)
	void Zoom(float ZoomDelta);

	UFUNCTION(BlueprintCallable)
	void ChangeAperture(float ApertureDelta);

	UPROPERTY(BlueprintReadOnly)
	float ZoomSpeed = 0.1f;

	UFUNCTION(BlueprintCallable)
	void EnterPhotoMode();

	UFUNCTION(BlueprintCallable)
	void ExitPhotoMode();

};
