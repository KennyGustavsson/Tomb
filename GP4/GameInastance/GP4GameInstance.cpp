#include "GP4GameInstance.h"

#include "Camera/CameraComponent.h"
#include "GP4/GP4Player.h"

void UGP4GameInstance::SetPlayerSensitivity(float Value, AGP4Player* Player)
{
	Player->MouseSensitivity = Value;
	Sensitivity = Value;
}

void UGP4GameInstance::SetPlayerFOV(float Value, AGP4Player* Player)
{
	Player->CameraComp->FieldOfView = Value;
	FOV = Value;
}

float UGP4GameInstance::GetSensitivity() const
{
	return Sensitivity;
}

float UGP4GameInstance::GetFOV() const
{
	return FOV;
}
