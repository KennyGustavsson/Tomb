#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElevatorPlatform.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAddForce);

UCLASS()
class GP4_API AElevatorPlatform : public AActor
{
	GENERATED_BODY()

public:
	AElevatorPlatform();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

// Components
	UPROPERTY(EditDefaultsOnly)
	USceneComponent* Root;
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* ElevatorMeshComponent;

// Functions
	UFUNCTION(BlueprintCallable)
	void AddForceToElevator();

// Events
	UPROPERTY(BlueprintAssignable)
	FOnAddForce OnAddForce;

// Variables
private:
	UPROPERTY(EditInstanceOnly)
	float MinZHeight = -800.0f;
	UPROPERTY(EditInstanceOnly)
	float MaxZHeight = 200.0f;
	UPROPERTY(EditInstanceOnly)
	float Force = 10.0f;
	UPROPERTY(EditInstanceOnly)
	float ZGravity = 6.0f;
	UPROPERTY(Transient)
	float ZVelocity = 0.0f;
	UPROPERTY(Transient)
	UPrimitiveComponent* ElevatorPrimitiveComponent;
};
