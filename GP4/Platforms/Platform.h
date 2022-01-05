#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platform.generated.h"

UENUM(Blueprintable)
enum EPlatformState
{
	Moving,
	Waiting,
	Rotating,
	MoveTowardsPointOnSpline,
	ResetToZero
};

UENUM(Blueprintable)
enum EEasing
{
	NoEasing,
	InOutSine,
	CustomCurve
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActivate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateFinnish);

UCLASS()
class GP4_API APlatform : public AActor
{
	GENERATED_BODY()

public:
	APlatform();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

// Functions
	void NextState();
	void Moving(float const DeltaTime);
	void Waiting(float const DeltaTime);
	void Rotating(float const DeltaTime);
	void MoveTowardsPointOnSpline(float const DeltaTime);
	void ResetToZero(float const DeltaTime);
	void ForceReset(float const DeltaTime);
	UFUNCTION(BlueprintCallable)
	void SetActive(bool const Active);
	UFUNCTION(BlueprintPure)
	bool IsActive() const { return bActive; }
	UFUNCTION(BlueprintCallable)
	void UpdateMoveTowardsPointOnSpline();
	UFUNCTION(BlueprintCallable)
	void ForceReset(bool AfterResetActive);
	
	static float EaseInOutSine(float const X);
	static float InverseLerp(float const A, float const B, float const Distance);
	float RemapStartStateAccumulator(float const End, float const T) const;
	float RemapWithEase(float const End, float const T) const;
	float RemapWithCurve(float const End, float const T) const;

// Components
	UPROPERTY(EditAnywhere)
	USceneComponent* Root;
	UPROPERTY(EditAnywhere)
	class UBoxComponent* Box;
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere)
	class USplineComponent* PlatformSpline;

// Spline movement
	UPROPERTY(EditAnywhere)
	UCurveFloat* SplineSpeedMultiplierCurve;
	UPROPERTY(EditAnywhere)
	float SplineTravelTime = 5.0f;
	UPROPERTY(Transient)
	float Accumulator = 0.0f;
	UPROPERTY(Transient)
	int Direction = 1;
	
// Waiting
	UPROPERTY(EditAnywhere)
	float WaitingTime = 5.0f;
	UPROPERTY(Transient)
	float WaitingAccumulator = 0.0f;
	
// Rotation
	UPROPERTY(EditAnywhere)
	FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere)
	float RotationTime = 2.5f;
	UPROPERTY(Transient)
	float RotationAccumulator = 0.0f;
	UPROPERTY(Transient)
	FQuat StartRot;
	UPROPERTY(Transient)
	FQuat EndRot;
	UPROPERTY(Transient)
	FQuat OriginalRotation;

// MoveTowardsPointOnSpline
	UPROPERTY(EditInstanceOnly)
	TArray<AActor*> MoveTowardsActorOnSpline;
	UPROPERTY(Transient)
	float MoveTowardsActorOnSplineKey = 0.0f;
	UPROPERTY(Transient)
	int CurrentActor = 0;

// State
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EPlatformState>> StateSequence;
	UPROPERTY(Transient)
	int CurrentState = 0;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EEasing> Easing = EEasing::NoEasing;
	UPROPERTY(EditAnywhere)
	bool bDoOnce = false;
	UPROPERTY(EditAnywhere)
	bool bActive = true;
	UPROPERTY(EditAnywhere)
	bool bSetDisableOnStateChange = false;
	UPROPERTY(Transient)
	float StartStateAccumulator = 0;
	UPROPERTY(Transient)
	bool bForceReset = false;
	UPROPERTY(Transient)
	bool bWasActive = false;
	UPROPERTY(Transient)
	bool bDeactivateAfterReset = false;
	UPROPERTY(BlueprintAssignable)
	FOnStateFinnish OnStateFinnish;
	UPROPERTY(BlueprintAssignable)
	FOnActivate OnActivate;
};
