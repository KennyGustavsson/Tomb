#pragma once

#include "GP4GameInstance.generated.h"

UCLASS()
class GP4_API UGP4GameInstance : public UGameInstance
{
	GENERATED_BODY()

// Functions
public:
	UFUNCTION(BlueprintCallable)
	void SetPlayerSensitivity(float Value, class AGP4Player* Player);
	UFUNCTION(BlueprintCallable)
	void SetPlayerFOV(float Value, class AGP4Player* Player);
	UFUNCTION(BlueprintCallable)
	float GetSensitivity() const;
	UFUNCTION(BlueprintCallable)
	float GetFOV() const;

// Variables
private:
	UPROPERTY(Transient)
	float Sensitivity = 1.0;
	UPROPERTY(Transient)
	float FOV = 100.0f;
};