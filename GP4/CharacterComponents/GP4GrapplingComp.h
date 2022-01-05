#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GP4GrapplingComp.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHookEvent);

UENUM(BlueprintType)
enum EDirection
{
	Up,
	Down
};

USTRUCT(BlueprintType)
struct FSpeedAndLocation
{
	GENERATED_USTRUCT_BODY()
	/*Bounce Direction*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location And Speed")
	TEnumAsByte<EDirection> Direction;
	/*length of bounce*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location And Speed")
	float LocationOffset = 0;
	/*speed of bounce*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location And Speed")
	float Speed = 4;
	FVector Location;
};


USTRUCT(BlueprintType)
struct FPlayerDestination
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bouncing Offset And Speed")
	TArray<FSpeedAndLocation> BouncingLocationAndSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bouncing Offset And Speed")
	float FinalBounceSpeed = 4;

	/* Bouncing Is Calculated Based on Percentage Player To Hinge ( How far Player is When Hooking ) */
	UPROPERTY(EditAnywhere, Category = "Bouncing Offset And Speed")
	bool bIsPercentageBased = false;

	/*Clamp Percentage of High bounce and low bounce*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Location And Speed",
		meta = (EditCondition = "bIsPercentageBased"))
	FVector2D ClampMinMaxDistance = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Bouncing Offset And Speed", meta =(InlineEditConditionToggle))
	bool bIsAngleLimit = false;
	/*Allow Bounce Only If Angle is less than MinAngle */
	UPROPERTY(EditAnywhere, Category = "Bouncing Offset And Speed", meta = (EditCondition = "bIsAngleLimit"))
	int32 MinAngle;

	UPROPERTY(EditAnywhere, Category = "Bouncing Offset And Speed", meta =(InlineEditConditionToggle))
	bool bIsDistanceLimit = false;
	/*Allow Bounce Only If Distance between Player and Hinge Higher than MinDistance*/
	UPROPERTY(EditAnywhere, Category = "Bouncing Offset And Speed", meta = (EditCondition = "bIsDistanceLimit"))
	int32 MinDistance;


	bool bIsFinalDestination = false;
	bool bIsPreCalculated = false;
	int CurrentIndex;
	float CableLength;
	void Reset();
	FSpeedAndLocation GetPathInfo();
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4_API UGP4GrapplingComp : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	class AGP4Player* PlayerCharacter;
	
	// -- General Settings

	/* Allow Player to hook when he is above the grappling point */
	UPROPERTY(EditDefaultsOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	bool bIsHookingDown = false;
	/* Toggle Debug On/Off */
	UPROPERTY(EditDefaultsOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	bool bIsDebug = false;
	/* Lerp Speed Player Toward the cable length range*/
	UPROPERTY(EditDefaultsOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	float InterSpeed = 4000;

	/*You can Swing left and right by keyboard keys*/
	UPROPERTY(EditDefaultsOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	bool bCanTurnWithKeys = false;

	// -- RayCast For Hooking //

	/* RayCast Radius */
	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings", meta = (AllowPrivateAccess = "true"))
	float RayCastRadius = 150;
	/* RayCast Start Location */
	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings", meta = (AllowPrivateAccess = "true"))
	float RayCastStartLocation = 200;
	/* RayCast End Location */
	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings", meta = (AllowPrivateAccess = "true"))
	float RayCastEndLocation = 1300;
	/* RayCast Offset, Mostly used to center the ray cast . Enable bIsDebug in general settings */
	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings", meta = (AllowPrivateAccess = "true"))
	FVector RayCastOffset = FVector(0, 0, 30);;
	/*Making the Swing faster , After the leap*/
	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings", meta = (AllowPrivateAccess = "true"))
	float Tolerance = 50.f;


	// -- Swing , Release , Jump Settings //

	/* Force axis input multiplier , higher number leads to higher swings but affect rope smoothness*/
	UPROPERTY(EditDefaultsOnly, Category="Swing / release Settings", meta = (AllowPrivateAccess = "true"))
	float SwingForce = 100;
	/*Speed of Turning while Swinging left and right only*/
	UPROPERTY(EditDefaultsOnly, Category="Swing / release Settings", meta = (AllowPrivateAccess = "true"))
	float TurnSpeed = 100;
	/* Determine how much you elevate when releasing the rope */
	UPROPERTY(EditDefaultsOnly, Category="Swing / release Settings", meta = (AllowPrivateAccess = "true"))
	float ReleaseJumpUpwardForce = 500;
	/* Determine how much you Forward force is Applied when releasing the rope */
	UPROPERTY(EditDefaultsOnly, Category="Swing / release Settings", meta = (AllowPrivateAccess = "true"))
	float ReleaseJumpForwardForce = 300;
	/* Player must grab rope for X seconds to activate a jump */
	UPROPERTY(EditDefaultsOnly, Category="Swing / release Settings", meta = (AllowPrivateAccess = "true"))
	float JumpTimeOffset = 0.3f;
	/*Manipulate Gravity for short period of time When releasing rope */
	UPROPERTY(EditDefaultsOnly, Category="Swing / release Settings", meta = (AllowPrivateAccess = "true"))
	float GravityGraceTimeJump = 0.5f;


	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray; // object types to trace
	ETraceTypeQuery MyTraceType; // Visibility Trace
	bool bIsHookPtAvailable = false;
	bool bIsAttached = false;
	bool bIsLeaping = false;
	float JumpOffsetTime;
	FRotator HingeInitialRotation;
	UPROPERTY(EditDefaultsOnly, Category="Bounce Settings", meta = (AllowPrivateAccess = "true"))
	FPlayerDestination PlayerDestination;
	UPROPERTY()
	class AGP4Hinge* Hinge;

public:
	UGP4GrapplingComp();

	UPROPERTY(BlueprintAssignable)
	FOnHookEvent OnHookEvent;
	void OnFire();
	void OnReleased();
	float GetPlayerRange() const;


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void CheckForHingePts();
	FVector GetHeadingLocation(float LengthFar) const;
	void MovePlayerInHingeDirection(FVector Location, float Speed, float DeltaTime);
	void BeginSwing();
	void GenerateLocations();
	float GetPlayerHingeDistance() const;
	void AddFinalTargetLocation(FVector FinalDest);
	void AttachPlayerToHinge() const;
	void DetachPlayerFromHinge() const;
};
