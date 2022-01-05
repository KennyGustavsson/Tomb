// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GP4MovementData.generated.h"

UCLASS()
class GP4_API UGP4MovementData : public UDataAsset
{
	GENERATED_BODY()
	
public:
//Basic Movement
	UPROPERTY(EditAnywhere)
	float MovementSpeed = 1000;
	UPROPERTY(EditAnywhere)
	float Friction = 0.9f;
	UPROPERTY(EditAnywhere)
	float SprintBoost = 2;

//Jumping
	UPROPERTY(EditAnywhere)
	float JumpForce = 9;
	UPROPERTY(EditAnywhere)
	int AmountOfJumps = 2;
	UPROPERTY(EditAnywhere)
	float AirControl = 0.025;
	UPROPERTY(EditAnywhere)
	float DoubleJumpMovementBoost = 2;
	UPROPERTY(EditAnywhere)
	float DoubleJumpTimeRestriction = 0.3f; //How long after jumping until you can double jump
	UPROPERTY(EditAnywhere)
	float AirFriction = 0.9f;
	
//Gravity
	UPROPERTY(EditAnywhere)
	float GravityScale = 0.25;
	UPROPERTY(VisibleAnywhere)
	TEnumAsByte<ETraceTypeQuery> GroundTrace = ETraceTypeQuery::TraceTypeQuery1; //Enforced Camera Channel
	UPROPERTY(EditAnywhere)
	float MaxSlopeInclination = 45; //The limit for how steep a slope can be before you can walk up it
	
//Wallrunning
	UPROPERTY(EditAnywhere)
	float WallRunningSpeed = 2000;
	UPROPERTY(EditAnywhere)
	float MinWallRunInclination = 75;
	UPROPERTY(EditAnywhere)
	float MaxWallRunInclination = 115;
	UPROPERTY(EditAnywhere)
	float WallRunJumpModifier = 1.5;
	UPROPERTY(EditAnywhere)
	float LookAlongWallLimit = 0.75;
	UPROPERTY(VisibleAnywhere)
	TEnumAsByte<ETraceTypeQuery> WallRunTrace = ETraceTypeQuery::TraceTypeQuery3; //Enforced Wall running Channel
	UPROPERTY(EditAnywhere)
	float WallRunDetachGracePeriod = 0.1f;
	UPROPERTY(EditAnywhere)
	float WallRunHitBoxGraceScale = 1.1f;
	UPROPERTY(EditAnywhere)
	UCurveFloat* WallRunZCurve;
	UPROPERTY(EditAnywhere)
	float CameraTiltDegrees = 30;
	UPROPERTY(EditAnywhere)
	float WallRunJumpGracePeriod = 0.2;
	

//Marouns Rope Utility
	UPROPERTY(EditAnywhere)
	float TempGravityScale;
};
