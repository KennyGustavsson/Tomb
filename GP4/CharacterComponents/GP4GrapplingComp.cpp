#include "GP4GrapplingComp.h"
#include "CableComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GP4/GP4Hinge.h"
#include "GP4/GP4MovementComponent.h"
#include "GP4/GP4Player.h"
#include "Kismet/KismetMathLibrary.h"


UGP4GrapplingComp::UGP4GrapplingComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGP4GrapplingComp::OnFire()
{
	// Send a rayCast And find a Rope.
	CheckForHingePts();
	if ((!bIsHookPtAvailable && !bIsAttached) || Hinge == nullptr) return;

	// Hook Found, Rotate Hinge Toward Player
	FRotator RotateTo = UKismetMathLibrary::FindLookAtRotation(PlayerCharacter->GetActorLocation(),
	                                                           Hinge->GetActorLocation());
	RotateTo = FRotator(RotateTo.Pitch - 90, RotateTo.Yaw, 0);
	Hinge->SetActorRotation(RotateTo);

	AttachPlayerToHinge();

	PlayerCharacter->MovementComponent->bIsPauseToLeap = true;
	bIsAttached = true;
	bIsLeaping = true;

	Hinge->Cable->SetAttachEndTo(PlayerCharacter, "", "");
	Hinge->Cable->bAttachEnd = true;
	OnHookEvent.Broadcast();
}

/*Attach Player To the Hinge*/
void UGP4GrapplingComp::AttachPlayerToHinge() const
{
	// Attach Player "Capsule" To Hook  
	const FAttachmentTransformRules TransformTransformRules(EAttachmentRule::KeepWorld,
	                                                        EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld,
	                                                        true);
	
	PlayerCharacter->HitBox->SetUsingAbsoluteRotation(true);
	PlayerCharacter->HitBox->AttachToComponent(Hinge->MobileMesh, TransformTransformRules, "");
}

/*Detach Player From Hinge*/
void UGP4GrapplingComp::DetachPlayerFromHinge() const
{
	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld,
	                                                         EDetachmentRule::KeepWorld, true);
	PlayerCharacter->HitBox->DetachFromComponent(DetachmentTransformRules);
}


/*On Releasing , We Detach the player from the Mobile Mesh of the Hinge
 * And we set a grace period for a smooth Gravity Free
 * Unlock the capsule orientation and reset the Hinge for the next Hook.
 */
void UGP4GrapplingComp::OnReleased()
{
	if (!bIsAttached) return;

	PlayerDestination.Reset();
	bIsAttached = false;
	bIsLeaping = false;
	PlayerCharacter->MovementComponent->bIsRopeSwing = false;
	PlayerCharacter->MovementComponent->bIsPauseToLeap = false;
	// Detach Player From Sphere and Add a Fwd Jump
	DetachPlayerFromHinge();
	if (UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld()) - JumpOffsetTime > JumpTimeOffset)
		PlayerCharacter->MovementComponent->RopeJump(GravityGraceTimeJump, ReleaseJumpUpwardForce,
		                                             PlayerCharacter->GetActorForwardVector() *
		                                             ReleaseJumpForwardForce);
	PlayerCharacter->HitBox->SetUsingAbsoluteRotation(false);
	Hinge->ResetHinge();
	Hinge = nullptr;
}

float UGP4GrapplingComp::GetPlayerRange() const
{
	return RayCastStartLocation + RayCastEndLocation + (RayCastRadius * 0.5f);
}


void UGP4GrapplingComp::BeginPlay()
{
	Super::BeginPlay();
	MyTraceType = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);

	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	PlayerCharacter = Cast<AGP4Player>(GetOwner());
	if (bCanTurnWithKeys) { PlayerCharacter->MovementComponent->bIsTurningWithKey = true; }
}

/*Tick is only Used to lerp the player towards the hinge
 * if player in range than we exit and beginSwing
 */
void UGP4GrapplingComp::TickComponent(float DeltaTime, ELevelTick Tick, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);

	if (Hinge == nullptr) return;

	Hinge->Cable->CableLength = (PlayerCharacter->GetActorLocation() - Hinge->GetActorLocation()).Size() - 300;


	// Leaping player towards Grappling Point. 
	if (bIsLeaping)
	{
		// Generate location High , Low And Target Destination Once
		GenerateLocations();
		if (PlayerDestination.bIsFinalDestination)
		{
			bIsLeaping = false;
			BeginSwing();
		}

		const FSpeedAndLocation Path = PlayerDestination.GetPathInfo();
		MovePlayerInHingeDirection(Path.Location, Path.Speed, DeltaTime);
	}
}

// Player in Cable Range and now rdy to start Swinging.
void UGP4GrapplingComp::BeginSwing()
{
	DetachPlayerFromHinge();
	Hinge->MobileMesh->SetSimulatePhysics(true);

	AttachPlayerToHinge();

	JumpOffsetTime = UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld());
	PlayerCharacter->MovementComponent->bIsRopeSwing = true;
	PlayerCharacter->MovementComponent->bIsPauseToLeap = false;
	PlayerCharacter->MovementComponent->StartRopeSwing(Hinge, SwingForce, TurnSpeed);
	Hinge->SetActorRotation(FRotator(0, Hinge->GetActorRotation().Yaw, 0));
	Hinge->SetWingLimit();
}

void UGP4GrapplingComp::GenerateLocations()
{
	if (!PlayerDestination.bIsPreCalculated)
	{
		FVector FinalDestination = PlayerCharacter->GetActorLocation();

		/* If Player is Jumping From Hook To another Within Cable range , Than he's at his right location
		* Else we calculate the new Position that we are going to go to .
		*/
		if ((PlayerCharacter->GetActorLocation() - Hinge->GetActorLocation()).Size() > Hinge->GetCableLength())
		{
			FinalDestination = GetHeadingLocation(Hinge->GetCableLength());
		}

		// Bouncing Cancel Conditions

		bool bIsBouncingCanceled = false;

		if (PlayerDestination.bIsDistanceLimit)
		{
			if (GetPlayerHingeDistance() <= FMath::Abs(PlayerDestination.MinDistance))
			{
				bIsBouncingCanceled = true;
			}
		}

		if (PlayerDestination.bIsAngleLimit)
		{
			const FRotator PlayerLooksAtHingeRotation = UKismetMathLibrary::FindLookAtRotation(
				PlayerCharacter->GetActorLocation(), Hinge->GetActorLocation());
			const float Angle = PlayerLooksAtHingeRotation.Pitch;
			if (Angle < PlayerDestination.MinAngle)
			{
				bIsBouncingCanceled = true;
			}
		}

		if (bIsBouncingCanceled)
		{
			AddFinalTargetLocation(FinalDestination);
			PlayerDestination.CurrentIndex = PlayerDestination.BouncingLocationAndSpeed.Num() - 1;
			PlayerDestination.bIsPreCalculated = true;
			return;
		}
		// Stop Checking Bounce Conditions


		for (int i = 0; i < PlayerDestination.BouncingLocationAndSpeed.Num(); i++)
		{
			FVector NewPreCalculatedLocation;
			// Make Offset Positive even if user put negative value
			float Offset = FMath::Abs(PlayerDestination.BouncingLocationAndSpeed[i].LocationOffset);
			// Becomes negative if he chose Direction Down 
			if (PlayerDestination.BouncingLocationAndSpeed[i].Direction == EDirection::Down)
				Offset = -Offset;

			if (PlayerDestination.bIsPercentageBased)
			{
				float Percentage = Hinge->GetCableLength() - ((PlayerCharacter->GetActorLocation() - Hinge->
					GetActorLocation()).Size() * Offset) / 100;
				const bool bIsNegative = Percentage < 0;
				Percentage = FMath::Clamp(FMath::Abs(Percentage), PlayerDestination.ClampMinMaxDistance.GetMin(),
				                          PlayerDestination.ClampMinMaxDistance.GetMax());
				NewPreCalculatedLocation = GetHeadingLocation(bIsNegative ? - Percentage : Percentage);
			}
			else
			{
				NewPreCalculatedLocation = GetHeadingLocation(
					Hinge->GetCableLength() - Offset);
			}
			PlayerDestination.BouncingLocationAndSpeed[i].Location = NewPreCalculatedLocation;
		}

		AddFinalTargetLocation(FinalDestination);
	}

	PlayerDestination.bIsPreCalculated = true;
}

float UGP4GrapplingComp::GetPlayerHingeDistance() const
{
	return (PlayerCharacter->GetActorLocation() - Hinge->GetActorLocation()).Size();
}

/*After all bounces are done , This is where the player will end*/
void UGP4GrapplingComp::AddFinalTargetLocation(FVector FinalDest)
{
	FSpeedAndLocation FinalSpeedAndLocation;
	FinalSpeedAndLocation.Location = FinalDest;
	FinalSpeedAndLocation.Speed = PlayerDestination.FinalBounceSpeed;
	PlayerDestination.BouncingLocationAndSpeed.Add(FinalSpeedAndLocation);
}


/* Apply SphereTraceMultiForObjects With these options
 * Can not hook to the same pt already swing on
 * hook when above the grappling pt " bIsHookingDown " Should be set to true To allow it 
 * Ensure that hook is within the Hook Range "GetHookDistance()"
 * Choose the closest hook to us if ray cast hits multi hooks at the same time.
*/
void UGP4GrapplingComp::CheckForHingePts()
{
	if (bIsAttached) return;
	const FVector CameraFwdVector = PlayerCharacter->CameraComp->GetForwardVector();
	const FVector StartLocation = PlayerCharacter->GetActorLocation() + RayCastOffset + (CameraFwdVector *
		RayCastStartLocation);
	const FVector EndLocation = PlayerCharacter->GetActorLocation() + RayCastOffset + (CameraFwdVector *
		RayCastEndLocation);

	TArray<FHitResult> Hits;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), StartLocation, EndLocation, RayCastRadius,
	                                                 ObjectTypesArray, false, TArray<AActor*>(),
	                                                 bIsDebug ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,
	                                                 Hits, true, FLinearColor::Red, FLinearColor::Green, 0);
	float Distance = 9999999;

	for (auto Hit : Hits)
	{
		if (Hit.bBlockingHit)
		{
			AGP4Hinge* HingeTemp = Cast<AGP4Hinge>(Hit.Actor);

			if (!bIsHookingDown) if (PlayerCharacter->GetActorLocation().Z > HingeTemp->GetActorLocation().Z) continue;
			if (Hinge != nullptr) if (HingeTemp == Hinge) continue;
			if (HingeTemp == nullptr || (PlayerCharacter->GetActorLocation() - HingeTemp->GetActorLocation()).Size() >
				HingeTemp->GetHookDistance() || !HingeTemp->IsAvailable())
				continue;

			FHitResult OneHit;

			UKismetSystemLibrary::LineTraceSingle(GetWorld(), PlayerCharacter->GetActorLocation() + RayCastOffset,
			                                      HingeTemp->GetActorLocation(),
			                                      MyTraceType, false, TArray<AActor*>(),
			                                      bIsDebug ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,
			                                      OneHit, true, FLinearColor::Red,
			                                      FLinearColor::Green, 1);
			if (OneHit.bBlockingHit)
			{
				AGP4Hinge* HingeTemp2 = Cast<AGP4Hinge>(OneHit.Actor);
				if (HingeTemp2 != HingeTemp)continue;
			}

			if (PlayerCharacter->GetDistanceTo(HingeTemp) < Distance)
			{
				Distance = PlayerCharacter->GetDistanceTo(HingeTemp);
				Hinge = HingeTemp;
			}
		}
	}

	bIsHookPtAvailable = Hinge != nullptr ? bIsHookPtAvailable = true : bIsHookPtAvailable = false;
}


/*Get Player in Range before allowing it to Swing
 * Player must get into GetCableLength() Range 
 */
FVector UGP4GrapplingComp::GetHeadingLocation(float LengthFar) const
{
	// Calculate New location of player Based on Same Direction as grappling point . 
	FVector Normal = (PlayerCharacter->GetActorLocation() - Hinge->GetActorLocation());
	Normal.Normalize();
	return Hinge->GetActorLocation() + (Normal * LengthFar);
}

void UGP4GrapplingComp::MovePlayerInHingeDirection(FVector Location, float Speed, float DeltaTime)
{
	const FVector InterLocation = UKismetMathLibrary::VInterpTo_Constant(PlayerCharacter->GetActorLocation(), Location,
	                                                                     DeltaTime, Speed * 1000);

	PlayerCharacter->SetActorLocation(InterLocation);

	if ((PlayerCharacter->GetActorLocation() - Location).IsNearlyZero(10))
	{
		PlayerDestination.CurrentIndex++;
		if (PlayerDestination.CurrentIndex == PlayerDestination.BouncingLocationAndSpeed.Num())
		{
			PlayerDestination.bIsFinalDestination = true;
		}
	}
}

void FPlayerDestination::Reset()
{
	bIsPreCalculated = false;
	bIsFinalDestination = false;
	BouncingLocationAndSpeed.RemoveAt(BouncingLocationAndSpeed.Num() - 1);
	CurrentIndex = 0;
}

/*Get Location We wanna go to , If Index not available get the Target Destination that was added by Script and not by designer
 * 	PlayerDestination.BouncingLocations.Add(FinalDestination); Was Added inside GenerateLocations() Method.
 */
FSpeedAndLocation FPlayerDestination::GetPathInfo()
{
	return CurrentIndex < BouncingLocationAndSpeed.Num()
		       ? BouncingLocationAndSpeed[CurrentIndex]
		       : BouncingLocationAndSpeed.Last(0);
}
