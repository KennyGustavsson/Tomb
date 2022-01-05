#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GP4Rope.generated.h"

UCLASS()
class GP4_API AGP4Rope : public AActor
{
	GENERATED_BODY()
	
	public:	
	AGP4Rope();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent *RootComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCableComponent *Cable;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UPhysicsConstraintComponent * PhysicsConstraintComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent *VisualGrappling;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent *EndCable;

	private:
	/* Actual Length of cable , If player hook from far distance he will be dragged / Lerp towards Grappling pt*/
	UPROPERTY(EditAnywhere, Category="Rope Settings", meta = (AllowPrivateAccess = "true"))
	float CableLength = 500;
	/* Minimum range allowed for Player Distance to hook */
	UPROPERTY(EditAnywhere, Category="Rope Settings", meta = (AllowPrivateAccess = "true"))
	float HookDistance = 800;
	/* If True we find the closest Platform and attach it to it. */

	public:
	UPROPERTY(EditInstanceOnly, Category="Rope Settings", meta = (AllowPrivateAccess = "true"))
	bool bIsConnectedToPlatform = false;
	

	
	FVector InitialPlatformBoxOffsetLocation ;

	FRotator InitialRotation;
	FTimerHandle CollisionTimer;
	FTimerHandle PositionUpdateTimer;
	void TurnOnCollision() const;
	void FollowPlatform();
	float HookRestPeriod = 0.3f;
	int SubCableLength = 150;
	float RefreshRateFollowPlatform = 0.02;
	
	UPROPERTY()
	class APlatform* Platform;

	
	protected:
	virtual void BeginPlay() override;
	public:	
	void ResetRope();
	void UpdateRope(float CableLength, FVector Location, bool bIsSimulatePhysics) const;
	float GetCableLength() const {return CableLength;};
	float GetHookDistance() const {return HookDistance;};
};
