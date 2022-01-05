// Fill out your copyright notice in the Description page of Project Settings.

#include "GP4MovementComponent.h"

#include "CableComponent.h"
#include "GP4Hinge.h"
#include "Components/CapsuleComponent.h"
#include "GP4MovementData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

UGP4MovementComponent::UGP4MovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGP4MovementComponent::BeginPlay()
{
	Super::BeginPlay();

	JumpsLeft = MovementData->AmountOfJumps;
	
	UActorComponent* UncastedHitbox = GetOwner()->GetComponentByClass(UCapsuleComponent::StaticClass());
	Hitbox = Cast<UCapsuleComponent>(UncastedHitbox);
	ensure(Hitbox != nullptr);
}

void UGP4MovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (bIsPauseToLeap) return;
	// If Rope is Used
	if(bIsRopeSwing)
	{
		HandleRopeSwing();
		return;
	}

	if(bIsWallRunning)
	{
		HandleWallRun(DeltaTime);
		return;
	}
	CheckWallRun();
	
	HandlePlatformMovement();
	bool bShouldApplyGravity = GroundCheck();

	if(bIsHoldingJump && !bHasJumped)
		HandleJump(DeltaTime);

	//Handle Gravity
	if(bGravityEnabled && bShouldApplyGravity)
	{
		HandleGravity(DeltaTime);
	}

	if(bAllowInputMovement)
		Velocity += HandleInput(DeltaTime);
	
	//reset inputs to next frame
	CachedMovementInput = MovementInput; //Cache of MovementInput
	MovementInput = FVector::ZeroVector;

	Velocity = ApplyFriction(DeltaTime, Velocity);
	
	//Velocity = Velocity.GetClampedToMaxSize2D(MovementData->MaxSpeed);
	//Velocity.Z = FMath::Min(Velocity.Z, MovementData->MaxSpeed);
	

	FHitResult Hit;
	SafeMoveUpdatedComponent(Velocity * DeltaTime, GetOwner()->GetActorRotation(), true, Hit);
	if(Hit.bBlockingHit)
	{
		SlideAlongSurface(Velocity * DeltaTime, 1.0 - Hit.Time, Hit.Normal, Hit);
	}
}

void UGP4MovementComponent::ReceiveMovementInput(const FVector Input)
{
	MovementInput += Input;
}

void UGP4MovementComponent::BeginSprint()
{
	bIsSprinting = true;
}

void UGP4MovementComponent::EndSprint()
{
	bIsSprinting = false;
}

FVector UGP4MovementComponent::GetWorldMovement()
{
	FVector Forward = FVector::VectorPlaneProject(GetOwner()->GetActorForwardVector(), FVector::UpVector);
	FVector ForwardMovement = Forward * MovementInput.X;
	FVector Right = FVector::CrossProduct(FVector::UpVector, Forward); //Relative right
	FVector RightMovement = Right * MovementInput.Y;
	return (ForwardMovement + RightMovement).GetSafeNormal();
}

FVector UGP4MovementComponent::HandleInput(float DeltaTime)
{
	FVector MovementDirection = GetWorldMovement();
	
	float Modifier = bIsMidAir ? MovementData->AirControl : 1;

	if(bIsSprinting)
		Modifier *= MovementData->SprintBoost;
	
	return MovementDirection * MovementData->MovementSpeed * Modifier * DeltaTime;
}

void UGP4MovementComponent::HandlePlatformMovement()
{
	if(bIsMidAir || PlatformBase == nullptr)
		return;

	FVector NewPlatLocation = PlatformBase->GetComponentLocation();
	FVector Delta = NewPlatLocation - OldPlatLocation;
	if(Delta.IsNearlyZero())
		return;

	FHitResult Hit;
	//no need for DeltaTime since we are just sticking to the platform
	MoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, &Hit);

	OldPlatLocation = NewPlatLocation;
}

bool UGP4MovementComponent::GroundCheck()
{
	FVector RaycastStart = Hitbox->GetComponentLocation();
	FVector RaycastEnd = RaycastStart + (FVector::DownVector * Hitbox->GetScaledCapsuleHalfHeight()*0.499f);
	FHitResult GroundHit;
	UKismetSystemLibrary::SphereTraceSingle(this, RaycastStart, RaycastEnd, Hitbox->GetScaledCapsuleRadius()*0.99f, MovementData->GroundTrace, true, TArray<AActor*>(), EDrawDebugTrace::None, GroundHit, true);
	
	if(GroundHit.bBlockingHit)
	{
		//This check improves collision with spinning platforms.
		ResolvePenetration(GetPenetrationAdjustment(GroundHit), GroundHit, GetOwner()->GetActorRotation());
	
		float GroundAngle = DegAnglesFromVector(FVector::UpVector, GroundHit.Normal);
		
		if(GroundAngle < MovementData->MaxSlopeInclination)
		{
			HandleOnGround(GroundHit);
			return false;
		}
		//else
		return true;
	}
	//else
	bIsMidAir = true;
	return true;
}

void UGP4MovementComponent::HandleOnGround(const FHitResult& Hit)
{
	if(bIsMidAir) //this is needed since raycast might still find ground the frame after jumping
	{
		JumpsLeft = MovementData->AmountOfJumps;
		bIsMidAir = false;
		OnJumpLand.Broadcast(Velocity.Z);
	}
	Velocity.Z = FMath::Max(0.f, Velocity.Z); //Make sure gravity does not keep accumulating and overflow while on ground

	PlatformBase = Hit.Component.Get();
	OldPlatLocation = PlatformBase->GetComponentLocation();
}

void UGP4MovementComponent::HandleGravity(float Deltatime)
{
	if(bUseRopeJumpGravity)
		Velocity += Gravity * MovementData->TempGravityScale * Deltatime;
	else
		Velocity += Gravity * MovementData->GravityScale * Deltatime;
}

void UGP4MovementComponent::CheckWallRun()
{
	if(bIsWallRunning || bInWallRunGrace || !bIsMidAir)
		return;

	FHitResult Hit;
	
	UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		GetOwner()->GetActorLocation(),
		GetOwner()->GetActorLocation(),
		Hitbox->GetScaledCapsuleRadius() * MovementData->WallRunHitBoxGraceScale,
		Hitbox->GetScaledCapsuleHalfHeight()* MovementData->WallRunHitBoxGraceScale,
		MovementData->WallRunTrace,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		Hit,
		true
		);

	if(!Hit.bBlockingHit)
		return;
	//else we hit a wall-runnable wall

	FVector Normal = Hit.Normal;
	
	Hit.Reset();
	FVector Start = GetOwner()->GetActorLocation();

	UKismetSystemLibrary::LineTraceSingle(
		this,
		Start,
		Start - (Normal * Hitbox->GetScaledCapsuleRadius() * MovementData->WallRunHitBoxGraceScale),
		MovementData->WallRunTrace,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		Hit,
		true
		);

	if(!Hit.bBlockingHit)
		return;

	float WallAngle = DegAnglesFromVector(FVector::UpVector, Hit.Normal);
	
	if(WallAngle > MovementData->MinWallRunInclination && WallAngle < MovementData->MaxWallRunInclination)
	{
		FVector PlayerFacing = GetOwner()->GetActorForwardVector();
		FVector Cross = FVector::CrossProduct(Hit.Normal, PlayerFacing).GetSafeNormal(); //this may go up or down along the wall depending on facing
		FVector AlongWall = FVector::CrossProduct(Cross, Hit.Normal).GetSafeNormal(); //this gives us a vector left or right along the wall, according to player facing

		FVector RightAlongWall = FVector::CrossProduct(Hit.Normal, FVector::UpVector).GetSafeNormal();
		FVector UpAlongWall = FVector::CrossProduct(RightAlongWall, Hit.Normal).GetSafeNormal();

		float AmountLookingAlongWall = FVector::DotProduct(PlayerFacing, AlongWall); //This makes sure you can wall run from looking straight at the wall
		if(AmountLookingAlongWall > MovementData->LookAlongWallLimit)
		{
			bIsWallRunning = true;
			WallNormal = Hit.Normal;
			WallRunDirection = AlongWall; //make sure we run into wall to keep triggering hits
			WallUpVector = UpAlongWall;
			JumpsLeft = 1;
			WallRunGravityTimer = 0;

			//Wall tilting

			float TiltDirection = FMath::Sign(FVector::DotProduct(WallNormal,GetOwner()->GetActorRightVector())); //-1 = Tilt Left, +1 = Tilt right
			CamTiltStart.Broadcast(TiltDirection, MovementData->CameraTiltDegrees);
		}
	}
}

//Handles the wall run movement in Tick
void UGP4MovementComponent::HandleWallRun(float DeltaTime)
{
	WallRunGravityTimer += DeltaTime;
	FVector WallRunMovement = (WallRunDirection-WallNormal) * MovementData->WallRunningSpeed * DeltaTime;
	WallRunMovement.Z = MovementData->WallRunZCurve->GetFloatValue(WallRunGravityTimer);
	
	FHitResult Hit;
	FVector Start = GetOwner()->GetActorLocation();
	Start.Z += Hitbox->GetScaledCapsuleHalfHeight();

	UKismetSystemLibrary::LineTraceSingle(
		this,
		Start,
		Start - (WallNormal * Hitbox->GetScaledCapsuleRadius() * MovementData->WallRunHitBoxGraceScale),
		MovementData->WallRunTrace,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		Hit,
		true
		);

	if(!Hit.bBlockingHit)
		WallRunMovement.Z = FMath::Min(WallRunMovement.Z, 0.f);

	Hit.Reset();
	Start = GetOwner()->GetActorLocation();
	FVector End = Start + WallRunMovement;
	
	UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		Start,
		End,
		Hitbox->GetScaledCapsuleRadius() * MovementData->WallRunHitBoxGraceScale,
		Hitbox->GetScaledCapsuleHalfHeight() * MovementData->WallRunHitBoxGraceScale,
		MovementData->GroundTrace,
		true,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		Hit,
		true);
	
	if(GetJumpPending())
	{
		bIsHoldingJump = false;
		HandleWallJumpOff();
	}
	else if(Hit.bBlockingHit && (WallNormal-Hit.Normal).IsNearlyZero())
	{
		SlideAlongSurface(WallRunMovement, 1.0 - Hit.Time, Hit.Normal, Hit);
	}
	else //Ran out of wall
	{
		bIsWallRunning = false;
		Velocity = WallRunDirection * MovementData->WallRunningSpeed;
		bHasJumped = true;

		StartWallRunJumpGraceTimer();

		CamTiltStop.Broadcast();
	}
}

void UGP4MovementComponent::StartWallRunGraceTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(WallRunGraceTimer);
	GetWorld()->GetTimerManager().SetTimer(WallRunGraceTimer,this, &UGP4MovementComponent::ResetWallRunGraceTimer, MovementData->WallRunDetachGracePeriod, false);
	bInWallRunGrace = true;
}

void UGP4MovementComponent::ResetWallRunGraceTimer()
{
	bInWallRunGrace = false;
}

void UGP4MovementComponent::StartWallRunJumpGraceTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(WallRunJumpGraceTimer);
	GetWorld()->GetTimerManager().SetTimer(WallRunJumpGraceTimer,this, &UGP4MovementComponent::ResetWallRunJumpGraceTimer, MovementData->WallRunJumpGracePeriod, false);
	bInWallRunJumpGrace = true;
}

void UGP4MovementComponent::ResetWallRunJumpGraceTimer()
{
	bInWallRunJumpGrace = false;
}

void UGP4MovementComponent::HandleWallJumpOff()
{
	Velocity = (WallRunDirection.GetSafeNormal() + WallNormal).GetSafeNormal() * MovementData->WallRunJumpModifier;
	Velocity.Z = MovementData->JumpForce;
	bIsWallRunning = false;
	StartWallRunGraceTimer();
	StartDoubleJumpTimer();
	OnJumpStart.Broadcast();
	CamTiltStop.Broadcast();
	bHasJumped = true;
}

float UGP4MovementComponent::DegAnglesFromVector(FVector v1, FVector v2)
{
	return FMath::RadiansToDegrees(RadAnglesFromVector(v1,v2));
}

float UGP4MovementComponent::RadAnglesFromVector(FVector v1, FVector v2)
{
	return RadAnglesFromDot(FVector::DotProduct(v1, v2));
}

float UGP4MovementComponent::RadAnglesFromDot(float Dot)
{
	return FMath::Acos(Dot);
}

void UGP4MovementComponent::PressJump()
{
	bIsHoldingJump = true;
	bHasJumped = false;
}

void UGP4MovementComponent::ReleaseJump()
{
	bIsHoldingJump = false;
}

void UGP4MovementComponent::HandleJump(float DeltaTime)
{
	if(bInWallRunJumpGrace)
	{
		HandleWallJumpOff();
		return;
	}
	
	bHasJumped = true;
	if(JumpsLeft > 0 && bAllowedToJump)
	{
		ClearTempGravity();
		
		Velocity.Z = MovementData->JumpForce;
		--JumpsLeft;

		if(bIsMidAir)
		{
			FVector MovementDirection = GetWorldMovement();
			float AmountFacingVelocity = RadAnglesFromVector(Velocity.GetSafeNormal2D(), MovementDirection.GetSafeNormal2D()) / PI;

			//No Deltatime since this is one frame only
			Velocity.X *= 1-AmountFacingVelocity;
			Velocity.Y *= 1-AmountFacingVelocity;

			Velocity += MovementDirection * MovementData->MovementSpeed * MovementData->DoubleJumpMovementBoost * AmountFacingVelocity;
        
			bJumpBoost = false;
		}
		
		StartDoubleJumpTimer();
		
		OnJumpStart.Broadcast();
	}
}

FVector UGP4MovementComponent::ApplyFriction(float DeltaTime, FVector MovementVector)
{
	float Friction = bIsMidAir ? MovementData->AirFriction : MovementData->Friction;
	
	MovementVector.X = MovementVector.X * FMath::Pow(Friction, DeltaTime);
	MovementVector.Y = MovementVector.Y * FMath::Pow(Friction, DeltaTime);
	
	return MovementVector;
}

void UGP4MovementComponent::StartDoubleJumpTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(DoubleJumpTimer);
	GetWorld()->GetTimerManager().SetTimer(DoubleJumpTimer,this, &UGP4MovementComponent::ResetDoubleJumpTimer, MovementData->DoubleJumpTimeRestriction, false);
	bAllowedToJump = false;
}

void UGP4MovementComponent::ResetDoubleJumpTimer()
{
	bAllowedToJump = true;
}

// Rope Code
void UGP4MovementComponent::StartRopeSwing(AGP4Hinge *HingeArg, float SwingForceArg, float TurnSpeedArg)
{
	Hinge = HingeArg;
	bIsRopeSwing = true;
	SwingForce = SwingForceArg;
	SwingTurnSpeed = TurnSpeedArg;
	JumpsLeft = 1;
}

void UGP4MovementComponent::HandleRopeSwing()
{
	Hinge->MobileMesh->AddForce( SwingForce *  MovementInput.X * Hinge->MobileMesh->GetUpVector() * -10000);
	if(bIsTurningWithKey)
	{
		Hinge->SetActorRotation(Hinge->GetActorRotation() + FRotator(0,1,0) * SwingTurnSpeed * MovementInput.Y);
	}
	CachedMovementInput = MovementInput; //Cache of MovementInput
	MovementInput = FVector::ZeroVector;
}

void UGP4MovementComponent::RopeJump(float GraceTime, float JumpForce, FVector JumpImpulse)
{
	JumpForce = JumpForce < 0 ? MovementData->JumpForce : JumpForce; //if Jumpforce is -1, use default
	Velocity = JumpImpulse;
	Velocity.Z = JumpForce;
	SetGravityGracePeriod(GraceTime);
}

void UGP4MovementComponent::GravityReturnToNormal()
{
	bUseRopeJumpGravity = false;
}

void UGP4MovementComponent::ClearTempGravity()
{
	bUseRopeJumpGravity = false;
	GetWorld()->GetTimerManager().ClearTimer(GravityTimer);
}

void UGP4MovementComponent::SetGravityGracePeriod(float GraceTime)
{
	ClearTempGravity();
	GetWorld()->GetTimerManager().SetTimer(GravityTimer, this, &UGP4MovementComponent::GravityReturnToNormal, GraceTime, false);
	bUseRopeJumpGravity = true;
}

//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, FString::Printf(TEXT("%f"), float));

// End Rope
