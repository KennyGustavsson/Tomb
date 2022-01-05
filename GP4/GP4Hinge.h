// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GP4Hinge.generated.h"

class UPhysicsConstraintComponent;
UCLASS()
class GP4_API AGP4Hinge : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGP4Hinge();
	
	private:
	/* Actual Length of cable , If player hook from far distance he will be dragged / Lerp towards Grappling pt*/
	UPROPERTY(EditAnywhere, Category="Rope Settings", meta = (AllowPrivateAccess = "true"))
	float CableLength = 800;
	/* Minimum range allowed for Player Distance to hook */
	UPROPERTY(EditAnywhere, Category="Rope Settings", meta = (AllowPrivateAccess = "true"))
	float HookDistance = 2500;
	/* If True we find the closest Platform and attach it to it. */
	UPROPERTY(EditAnywhere, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	TSubclassOf <AGP4Hinge> NewHinge;
	
	FVector HingeLocation ;
	FTimerHandle CollisionTimer;
	FTimerHandle SwingLimitTimer;

	float HookRestPeriod = 1.f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent * RootScene ;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UPhysicsConstraintComponent * PhysicsConstraintComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent * FixMesh ;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent * MobileMesh ;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UBoxComponent * BoxCollision ;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCableComponent *Cable;

	float GetCableLength() const {return CableLength;};
	UFUNCTION(BlueprintCallable)
	float GetHookDistance() const {return HookDistance;};
	bool IsAvailable() const;
	void ResetHinge();
	void SetWingLimit();
	void ChangeAngularLimit() const;

	UFUNCTION(BlueprintPure)
	float GetPlayerRange() const;
	
};

