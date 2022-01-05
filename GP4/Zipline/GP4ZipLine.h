// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GP4ZipLine.generated.h"

UCLASS()
class GP4_API AGP4ZipLine : public AActor
{
	GENERATED_BODY()
	
public:	

	AGP4ZipLine();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class USplineComponent * SplineZipperLine ;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent *RootScene;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMesh * PipeMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UBoxComponent *StartCollider;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
public:
	UFUNCTION(BlueprintCallable)
	void StartSliding();
	virtual void Tick(float DeltaTime) override;
	
private:
	/*Speed of sliding on this specific Slide, Lower value slower slide*/
	UPROPERTY(EditInstanceOnly, Category="Settings", meta = (AllowPrivateAccess = "true"))
	float SlidingSpeed = 0.5f;
	/* Player Offset When sliding with zipper line */
	UPROPERTY(EditInstanceOnly, Category="Settings", meta = (AllowPrivateAccess = "true"))
	FVector PlayerOffset = FVector(0,0,-100);

	UFUNCTION()
	void OnPlayerJump();
	
	UPROPERTY()
	class AGP4Player* Player;
	float TempDistance = 0;
	bool bIsSliding = false;
	float ZipperLength = 0;

	/* Determine how much you elevate when releasing the rope */
	UPROPERTY(EditDefaultsOnly, Category="release Settings", meta = (AllowPrivateAccess = "true"))
	float ReleaseJumpUpwardForce = 500;
	/* Determine how much you Forward force is Applied when releasing the rope */
	UPROPERTY(EditDefaultsOnly, Category="release Settings", meta = (AllowPrivateAccess = "true"))
	float ReleaseJumpForwardForce = 300;
	/* Player must grab rope for X seconds to activate a jump */
	UPROPERTY(EditDefaultsOnly, Category="release Settings", meta = (AllowPrivateAccess = "true"))
	float JumpTimeOffset = 0.3f;
	/*Manipulate Gravity for short period of time When releasing rope */
	UPROPERTY(EditDefaultsOnly, Category="release Settings", meta = (AllowPrivateAccess = "true"))
	float GravityGraceTimeJump = 0.5f;
};
