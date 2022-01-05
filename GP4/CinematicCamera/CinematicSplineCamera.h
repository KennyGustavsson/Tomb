#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CinematicSplineCamera.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurrentEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNext);

UCLASS()
class GP4_API ACinematicSplineCamera : public AActor
{
	GENERATED_BODY()

public:
	ACinematicSplineCamera();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void OnNextIndex();
	void SetTickActive();

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;
	
	UPROPERTY(EditInstanceOnly)
	TArray<AActor*> SplineActors;
	UPROPERTY(Transient)
	TArray<class USplineComponent*> Splines;
	UPROPERTY(EditInstanceOnly)
	TArray<AActor*> LookAtActors;
	UPROPERTY(EditInstanceOnly)
	TArray<float> Speed;

	UPROPERTY(BlueprintAssignable)
	FOnCurrentEnd CurrentEnd;
	UPROPERTY(BlueprintAssignable)
	FOnNext OnNext;

private:
	UPROPERTY(EditInstanceOnly)
	float DefaultSpeed = 3.0f;
	UPROPERTY(EditAnywhere)
	float KeyDistance = 50.0f;
	UPROPERTY(EditAnywhere)
	float TimeBetweenSplines = 2.0f;
	
	int Index = 0;
};
