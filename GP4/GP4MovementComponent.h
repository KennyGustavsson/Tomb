// Hugo Lindroth 2021 FutureGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "GP4MovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLandSignature, float, ZVelocity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCamTiltStartSignature, float, TiltDirection, float, TiltAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCamTiltStopSignature);

class UCapsuleComponent;
class UGP4MovementData;
class AGP4Hinge;

UCLASS()
class GP4_API UGP4MovementComponent : public UMovementComponent
{
	GENERATED_BODY()

//Built in functions
public:
	UGP4MovementComponent();
protected:
	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

//Data Asset
public:
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
UGP4MovementData* MovementData;

//Links
protected:
	UCapsuleComponent* Hitbox;

//Basic Movement
public:
	void ReceiveMovementInput(const FVector Input);
	bool bAllowInputMovement = true;
	void BeginSprint();
	void EndSprint();
	UFUNCTION(BlueprintGetter)
	FVector GetCachedMovementInput(){return CachedMovementInput;}
	UFUNCTION(BlueprintGetter)
	FVector GetMovementInput(){return MovementInput;}
protected:
	UPROPERTY(BlueprintGetter=GetCachedMovementInput)
	FVector CachedMovementInput; //Since MovementInput is wiped in tick and it is needed later, we cache it here
	UPROPERTY(BlueprintGetter=GetMovementInput)
	FVector MovementInput;
	FVector GetWorldMovement();
	bool bIsSprinting;

//Velocity
public:
	float GetZVelocity(){return Velocity.Z;}
protected:
	FVector HandleInput(float DeltaTime);
	FVector ApplyFriction(float DeltaTime, FVector MovementVector);

//Jumping
public:
	void PressJump();
	void ReleaseJump();
	void SetJumpsLeft(int Value){JumpsLeft = Value;}
	int GetJumpsLeft(){return JumpsLeft;}
protected:
	void HandleJump(float DeltaTime);
	int JumpsLeft;
	bool bIsMidAir = false;
	bool bJumpBoost = false; //This is a trigger to give the double jump a boost in the other direction
	bool bIsHoldingJump = false;
	bool bHasJumped = false; //Is used to check that we haven't jumped this press of the button
//Double Jump Timer
	bool bAllowedToJump = true;
	bool GetJumpPending() { return bIsHoldingJump && !bHasJumped; }
	FTimerHandle DoubleJumpTimer;
	void StartDoubleJumpTimer();
	void ResetDoubleJumpTimer();
	
//Jump Events
public:
	UPROPERTY(BlueprintAssignable)
	FJumpSignature OnJumpStart;
	UPROPERTY(BlueprintAssignable)
	FLandSignature OnJumpLand;

//Based Movement (Platforms)
	UPROPERTY()
	UPrimitiveComponent* PlatformBase;
	FVector OldPlatLocation;
	void HandlePlatformMovement();

//Gravity
protected:
	const FVector Gravity =  FVector::DownVector * 9820.f;
	bool GroundCheck();
	void HandleOnGround(const FHitResult& Hit);
	bool bGravityEnabled = true;
	void HandleGravity(float Deltatime);

//Wall running
public:
	UPROPERTY(BlueprintReadOnly)
	bool bIsWallRunning = false;
protected:
	void CheckWallRun();
	void HandleWallRun(float DeltaTime);
	FVector WallRunDirection;
	FVector WallNormal;
	FVector WallUpVector;
	void HandleWallJumpOff();
	float WallRunGravityTimer;
//Grace Timers
	//bool bInWallRunGrace = false;
	bool bInWallRunGrace = false;
	void StartWallRunGraceTimer();
	void ResetWallRunGraceTimer();
	FTimerHandle WallRunGraceTimer;
	bool bInWallRunJumpGrace = false;
	void StartWallRunJumpGraceTimer();
	void ResetWallRunJumpGraceTimer();
	FTimerHandle WallRunJumpGraceTimer;
//Camera Tilt
	UPROPERTY(BlueprintAssignable)
	FCamTiltStartSignature CamTiltStart;
	UPROPERTY(BlueprintAssignable)
	FCamTiltStopSignature CamTiltStop;

//Utility
protected:
	float DegAnglesFromVector(FVector v1, FVector v2);
	float RadAnglesFromVector(FVector v1, FVector v2);
	float RadAnglesFromDot(float Dot);

//Marouns Rope Swing
public:
	void SetGravityGracePeriod(float GraceTime);
	void RopeJump(float GraceTime, float JumpForce = -1, FVector JumpImpulse = FVector::ZeroVector);
	bool bIsPauseToLeap = false;
	void StartRopeSwing(AGP4Hinge *RopeArg, float SwingForceArg, float TurnSpeed);
	bool bIsRopeSwing = false;
	bool bIsTurningWithKey = false;
protected:
	UFUNCTION()
	void GravityReturnToNormal();
	bool bUseRopeJumpGravity;
	FTimerHandle GravityTimer;
	void ClearTempGravity();
	UPROPERTY()
	AGP4Hinge *Hinge;
	void HandleRopeSwing();
	float SwingForce;
	float SwingTurnSpeed;
};
