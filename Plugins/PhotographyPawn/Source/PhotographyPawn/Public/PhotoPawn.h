// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/Scene.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PhotoPawn.generated.h"

class UCameraComponent;
class USceneCaptureComponent2D;

USTRUCT(BlueprintType)
struct FRealCameraLens
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	float MinFocalLengthMM = 24.f;
	
	UPROPERTY(BlueprintReadWrite)
	float MaxFocalLengthMM = 70.f;
};

UCLASS()
class PHOTOGRAPHYPAWN_API APhotoPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APhotoPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
	USceneCaptureComponent2D* SceneCapture;
	
	
	/**
	 * These settings will override the camera and scene capture post process settings. This variable will be changed
	 * when the player updates the camera settings in-game. 
	 */
	UPROPERTY(EditAnywhere)
	FPostProcessSettings CameraSettings;

	UPROPERTY(EditAnywhere)
	FRealCameraLens CameraLens;

	UPROPERTY(EditAnywhere)
	float LensFocalLengthMM;

	UFUNCTION()
	float FocalLengthtoFOV(float FocalLength);
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void CapturePhoto();

	UFUNCTION(BlueprintCallable)
	void Zoom(float ZoomDelta);

	UPROPERTY(BlueprintReadOnly)
	float ZoomSpeed = 0.1f;

};
