#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lava.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4_API ALava : public AActor
{
	GENERATED_BODY()

public:
	ALava();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

// Functions
	UFUNCTION()
	void SetLavaLevel(float LavaTime);
	UFUNCTION(BlueprintCallable)
	void SetEnabled(bool Enabled);

// Variables
	UPROPERTY(Transient)
	APawn* Player;
	UPROPERTY(Transient)
	float Timer = 0;
	UPROPERTY(Transient)
	float ZHeight = 0;
	UPROPERTY(EditAnywhere)
	UCurveFloat* LavaCurve;
	UPROPERTY(Transient)
	bool bEnabled = true;
};

