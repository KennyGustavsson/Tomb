#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraBobbingComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4_API UCameraBobbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraBobbingComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Functions
	public:
	UFUNCTION(BlueprintCallable)
	void HeadBob();
	UFUNCTION(BlueprintCallable)
	void LandingImpact(float ImpactForce);
	UFUNCTION(BlueprintCallable)
	void JumpingBob();
	UFUNCTION()
	void HeadBobLerp(float DeltaTime);
	UFUNCTION()
	void LandingImpactLerp(float DeltaTime);
	UFUNCTION()
	void JumpingBobLerp(float DeltaTime);
	UFUNCTION()
	void SetRelativeLocationBlend();
	
	// Variables
	public:
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* HeadBobCurve;
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* LandingImpactCurve;
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* JumpBobCurve;
	
	private:
	UPROPERTY(EditDefaultsOnly)
	float HeadBobTime = 0.5f;
	UPROPERTY(EditDefaultsOnly)
	float LandingImpactTime = 1.0f;
	UPROPERTY(EditDefaultsOnly)
	float JumpBobTime = 0.3f;
	
	UPROPERTY(Transient)
	class UCameraComponent* CameraComponent;
	UPROPERTY(Transient)
	FVector CameraOriginalRelativeLocation;

	UPROPERTY(Transient)
	bool bHeadBob = false;
	UPROPERTY(Transient)
	bool bLandingImpact = false;
	UPROPERTY(Transient)
	bool bJumpBob = false;
	UPROPERTY(Transient)
	float CurrentImpactForce = 0.0f;
	
	UPROPERTY(Transient)
	float HeadBobAccumulator = 0.0f;
	UPROPERTY(Transient)
	float LandingImpactAccumulator = 0.0f;
	UPROPERTY(Transient)
	float JumpBobAccumulator = 0.0f;

	UPROPERTY(Transient)
	float CurrentZ = 0.0f;
};
