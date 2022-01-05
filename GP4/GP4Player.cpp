#include "GP4Player.h"
#include "Components/CapsuleComponent.h"
#include "GP4MovementComponent.h"
#include "Camera/CameraComponent.h"
#include "CharacterComponents/GP4GrapplingComp.h"
#include "GameInastance/GP4GameInstance.h"

AGP4Player::AGP4Player()
{
	PrimaryActorTick.bCanEverTick = false;
	
	HitBox = CreateDefaultSubobject<UCapsuleComponent>("HitBox");
	//Default values for UE4 characters
	HitBox->SetCapsuleHalfHeight(96);
	HitBox->SetCapsuleRadius(55);
	RootComponent = HitBox;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("TempHandsMesh");
	MeshComp->SetupAttachment(RootComponent);
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComp->bUsePawnControlRotation = true;
	CameraComp->SetupAttachment(RootComponent);

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;

	MovementComponent = CreateDefaultSubobject<UGP4MovementComponent>("MovementComponent");
	GrapplingComp = CreateDefaultSubobject<UGP4GrapplingComp>(TEXT("Grappling Component"));

}

void AGP4Player::BeginPlay()
{
	Super::BeginPlay();

	UGP4GameInstance* GameInstance = Cast<UGP4GameInstance>(GetWorld()->GetGameInstance());
	if(GameInstance)
	{
		MouseSensitivity = GameInstance->GetSensitivity();
		CameraComp->FieldOfView = GameInstance->GetFOV();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GP4GameInstance not found"));
	}
}

void AGP4Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Basic Movement Controls
	PlayerInputComponent->BindAxis("MoveForward", this, &AGP4Player::InputMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGP4Player::InputMoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AGP4Player::InputPitch);
	PlayerInputComponent->BindAxis("Turn", this, &AGP4Player::InputTurn);

	//Additional Movement Controls
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGP4Player::InputJumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AGP4Player::InputJumpRelease);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AGP4Player::InputSprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AGP4Player::InputSprintRelease);
	PlayerInputComponent->BindAction("Grapple", IE_Pressed, GrapplingComp,  &UGP4GrapplingComp::OnFire);
	PlayerInputComponent->BindAction("Grapple", IE_Released, GrapplingComp,  &UGP4GrapplingComp::OnReleased);
}

void AGP4Player::InputMoveForward(float Value)
{
	MovementComponent->ReceiveMovementInput(Value * FVector::ForwardVector);
}

void AGP4Player::InputMoveRight(float Value)
{
	MovementComponent->ReceiveMovementInput(Value * FVector::RightVector);
}

void AGP4Player::InputJumpPressed()
{
	OnJumpEvent.Broadcast();
	MovementComponent->PressJump();
}

void AGP4Player::InputJumpRelease()
{
	MovementComponent->ReleaseJump();
}

void AGP4Player::InputSprintPressed()
{
	MovementComponent->BeginSprint();
}

void AGP4Player::InputSprintRelease()
{
	MovementComponent->EndSprint();
}

void AGP4Player::InputTurn(float Val)
{
	if(bMouseLookEnabled)
	{
		AddControllerYawInput(Val * MouseSensitivity);
		CachedMouseInput.X = Val;
	}
}

void AGP4Player::InputPitch(float Val)
{
	if(bMouseLookEnabled)
	{
		AddControllerPitchInput(Val * MouseSensitivity);
		CachedMouseInput.Y = Val;
	}
}

void AGP4Player::LockCamera()
{
	bMouseLookEnabled = false;
}

void AGP4Player::UnlockCamera()
{
	bMouseLookEnabled = true;
}

