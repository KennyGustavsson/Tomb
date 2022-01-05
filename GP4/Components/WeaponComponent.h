#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class AWeapon;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class GP4_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

// Functions
	UFUNCTION(BlueprintCallable)
	void SpawnWeaponAttachedToComponent(USceneComponent* Component, FVector Offset = FVector::ZeroVector);
	UFUNCTION(BlueprintCallable)
	void FireWeapon();
	UFUNCTION(BlueprintCallable)
	void StartPullUpWeapon();
	UFUNCTION(BlueprintCallable)
	void StartWeaponRecoil();
	UFUNCTION()
	void PullUpWeapon(float const DeltaTime);
	UFUNCTION()
	void WeaponRecoil(float const DeltaTime);
	UFUNCTION()
	void SetBlend();

// Variables
	UPROPERTY(EditDefaultsOnly, Category=WeaponComponent)
	TSubclassOf<AWeapon> Weapon;
	UPROPERTY(Transient)
	AWeapon* WeaponActor;
	UPROPERTY(Transient)
	FVector CurrentLoc;
	UPROPERTY(Transient)
	FRotator CurrentRot;

	// Weapon pull up
	UPROPERTY(EditDefaultsOnly)
	FVector PullUpWeaponStartRelLoc;
	UPROPERTY(EditDefaultsOnly)
	FRotator PullUpWeaponStartRelRot;
	UPROPERTY(Transient)
	float PullUpAccumulator = 0.0f;
	UPROPERTY(Transient)
	float PullUpTime = 1.0f;
	UPROPERTY(Transient)
	bool bPullUpWeapon = false;
	
	// Recoil
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilCurve;
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* RecoilXCurve;
	UPROPERTY(Transient)
	float RecoilAccumulator = 0.0f;
	UPROPERTY(Transient)
	float RecoilTime = 0.5f;
	UPROPERTY(Transient)
	bool bRecoil = false;
	
//Weapon ambient sway
protected:
	void HandleAmbientSway(float DeltaTime);
	float AmbientSwayTimer = 0;
	FVector AmbientToRotationVector;
	FRotator CurrentAmbientSway = FRotator::ZeroRotator; //This is a "layer" of Rotation imposed on CurrentRot 
//Tweakable Variables
public:
	UPROPERTY(EditDefaultsOnly)
	float AmbientSwayAmount = 1.f;
	UPROPERTY(EditDefaultsOnly)
	float AmbientSwaySpeed = 1.f;
	UPROPERTY(EditDefaultsOnly)
	float AmbientSwayTolerance = 1.f;

//Weapon Side and Forward Movement Sway
protected:
	void HandleSideSway(float DeltaTime);
	UPROPERTY(BlueprintSetter=SetMovementInput)
	FVector2D MovementInput;
	FVector CurrentForwardSway = FVector::ZeroVector;
	FVector CurrentSideSway = FVector::ZeroVector;
public:
	UFUNCTION(BlueprintSetter)
	void SetMovementInput(FVector2D MovementInputVector){MovementInput = MovementInputVector;}
	UPROPERTY(EditDefaultsOnly)
	float SideSwayAmount = 1.f;
	UPROPERTY(EditDefaultsOnly)
	float SideSwaySpeed = 1.f;

//Weapon Mouse Look Sway
protected:
	void HandleMouseSway(float DeltaTime);
	UPROPERTY(BlueprintSetter=SetMouseInput)
	FVector2D MouseInput;
	float CurrentPitch;
	float CurrentYaw;
public:
	UFUNCTION(BlueprintSetter)
    void SetMouseInput(FVector2D MouseInputVector){MouseInput = MouseInputVector;}
	UPROPERTY(EditDefaultsOnly)
	float MouseSwayAmount = 100.f;
	UPROPERTY(EditDefaultsOnly)
	float MouseSwaySpeed = 1.f;

//Landing Impact Bob
protected:
	void HandleLandingImpact(float DeltaTime);
	bool bLandingImpact = false;
	float LandingImpactTimer = 0.f;
	float ImpactForce = 0.f;
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* LandingBobCurve;
	UPROPERTY(EditDefaultsOnly)
	float LandingBobTime = 1.f;
public:
	UFUNCTION(BlueprintCallable)
	void StartLandingBob(float NewImpactForce);
};
