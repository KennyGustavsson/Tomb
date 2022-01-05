//Hugo Lindroth 2021 FutureGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GP4Player.generated.h"

class UGP4GrapplingComp;
class UCapsuleComponent;
class UCameraComponent;
class UGP4MovementComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumpEvent);

UCLASS()
class GP4_API AGP4Player : public APawn
{
	GENERATED_BODY()
//Built in functions
public:
	AGP4Player();
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

//Components
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCapsuleComponent* HitBox;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UGP4MovementComponent* MovementComponent;
	UPROPERTY(VisibleAnywhere)
	UGP4GrapplingComp *GrapplingComp;
	
//Input methods
protected:
	//Movement
	UFUNCTION()
	void InputMoveForward(float Value);
	UFUNCTION()
	void InputMoveRight(float Value);
	UFUNCTION()
	void InputJumpPressed();
	UFUNCTION()
	void InputJumpRelease();
	UFUNCTION()
	void InputSprintPressed();
	UFUNCTION()
    void InputSprintRelease();
	
	//Cached Input
	UPROPERTY(BlueprintGetter=GetCachedMouseInput)
	FVector2D CachedMouseInput = FVector2D::ZeroVector;
	
	//Camera handled with pawn rotation
	UFUNCTION()
    void InputTurn(float Val);
	UFUNCTION()
    void InputPitch(float Val);
	bool bMouseLookEnabled = true;
public:
	UPROPERTY(Transient)
	float MouseSensitivity = 1.0f;
	UFUNCTION(BlueprintGetter)
	FVector2D GetCachedMouseInput(){return CachedMouseInput;}

//Extra functions
public:
	UFUNCTION()
	void LockCamera();
	UFUNCTION()
    void UnlockCamera();
	UPROPERTY()
	FOnJumpEvent OnJumpEvent;
};
