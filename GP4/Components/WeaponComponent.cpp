#include "WeaponComponent.h"
#include "GP4/Weapon/Weapon.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick Tick, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);

	if(bPullUpWeapon)
	{
		PullUpWeapon(DeltaTime);
	}

	if(bRecoil)
	{
		WeaponRecoil(DeltaTime);
	}

	if(bLandingImpact)
	{
		HandleLandingImpact(DeltaTime);
	}

	HandleAmbientSway(DeltaTime);
	HandleSideSway(DeltaTime);
	HandleMouseSway(DeltaTime);

	SetBlend();
}

void UWeaponComponent::SpawnWeaponAttachedToComponent(USceneComponent* Component, FVector Offset)
{
	if(!Weapon)
	{
		UE_LOG(LogTemp, Error, TEXT("No weapon found to spawn"));
		return;
	}
	
	auto* Wep = GetWorld()->SpawnActor<AWeapon>(Weapon);
	Wep->AttachToComponent(Component, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Wep->AddActorLocalOffset(Offset);
	Wep->SetPlayer(GetOwner());
	WeaponActor = Wep;
}

void UWeaponComponent::FireWeapon()
{
	if(!WeaponActor)
	{
		UE_LOG(LogTemp, Error, TEXT("No weapon was initiated"));
		return;
	}

	WeaponActor->Fire();
}

void UWeaponComponent::StartPullUpWeapon()
{
	PrimaryComponentTick.SetTickFunctionEnable(true);
	bPullUpWeapon = true;

	if(PullUpAccumulator >= 1.0f)
	{
		PullUpAccumulator = 0.0f;
	}
}

void UWeaponComponent::StartWeaponRecoil()
{
	PrimaryComponentTick.SetTickFunctionEnable(true);
	bRecoil = true;

	if(RecoilAccumulator >= 1.0f)
	{
		RecoilAccumulator = 0.0f;
	}
}

void UWeaponComponent::PullUpWeapon(float const DeltaTime)
{
	const float Current = DeltaTime / PullUpTime;
	PullUpAccumulator += Current;
	PullUpAccumulator = PullUpAccumulator > 1.f ? 1.f : PullUpAccumulator;
	
	CurrentRot += FMath::Lerp(PullUpWeaponStartRelRot, FRotator::ZeroRotator, PullUpAccumulator);
	CurrentLoc += FMath::Lerp(PullUpWeaponStartRelLoc, FVector::ZeroVector, PullUpAccumulator);
	
	if(PullUpAccumulator >= 1.0f)
	{
		bPullUpWeapon = false;
	}
}

void UWeaponComponent::WeaponRecoil(float const DeltaTime)
{
	const float Current = DeltaTime / RecoilTime;
	RecoilAccumulator += Current;
	RecoilAccumulator = RecoilAccumulator > 1.f ? 1.f : RecoilAccumulator;

	if(RecoilCurve)
	{
		CurrentRot.Pitch += RecoilCurve->GetFloatValue(RecoilAccumulator);
	}

	if(RecoilXCurve)
	{
		CurrentLoc.X += RecoilXCurve->GetFloatValue(RecoilAccumulator);
	}
	
	if(RecoilAccumulator >= 1.0f)
	{
		bRecoil = false;
	}
}

void UWeaponComponent::SetBlend()
{
	if(!WeaponActor)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}
	
	WeaponActor->SetActorRelativeLocation(CurrentLoc);
	WeaponActor->SetActorRelativeRotation(CurrentRot);

	CurrentLoc = FVector::ZeroVector;
	CurrentRot = FRotator::ZeroRotator;

	if(!bPullUpWeapon && !bRecoil)
	{
		//PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}

void UWeaponComponent::HandleAmbientSway(float DeltaTime)
{
	if(!WeaponActor)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}
	
	AmbientSwayTimer += DeltaTime;

	//this calculation has to be done in Tick due to SwayAmount changing from frame to frame
	FVector CurrentTo = AmbientToRotationVector * AmbientSwayAmount;
	FVector DesiredLocalRotationDirection = CurrentTo + FVector::ForwardVector;
	FRotator DesiredLocalRotation = DesiredLocalRotationDirection.Rotation();
	
	FRotator RotationPerFrame = FMath::Lerp(CurrentAmbientSway, DesiredLocalRotation, AmbientSwayTimer*AmbientSwaySpeed);
	CurrentAmbientSway = RotationPerFrame;
	
	CurrentRot += CurrentAmbientSway;
	
	if((DesiredLocalRotation-CurrentAmbientSway).IsNearlyZero(AmbientSwayTolerance)) //0.01f = kinda low number
	{
		FVector2D NewV2D = FMath::RandPointInCircle(1);
		NewV2D.Normalize();
		
		int SignNewV2D = FMath::Sign(NewV2D.Y);
		int SignOldV2D = FMath::Sign(AmbientToRotationVector.Z); //Y is up in our 2d space, but Z is up in UE 3d space

		if(SignNewV2D == SignOldV2D)
			NewV2D.Y *= -1; //flip sign if its the same. This ensures constant movement up and down
		
		AmbientToRotationVector = FVector(0, NewV2D.X, NewV2D.Y);
		AmbientSwayTimer = 0;
	}
}

void UWeaponComponent::HandleSideSway(float DeltaTime)
{
	if(!WeaponActor)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}
	
	FVector DesiredForwardSway = FVector::ForwardVector * -MovementInput.X * SideSwayAmount; //Use same value as for side sway for easy tweaking
	FVector ForwardSwayDelta = FMath::Lerp(CurrentForwardSway, DesiredForwardSway, SideSwaySpeed * DeltaTime);
	CurrentForwardSway = ForwardSwayDelta;

	FVector DesiredSideSway = FVector::RightVector * -MovementInput.Y * SideSwayAmount;
	FVector SideSwayDelta = FMath::Lerp(CurrentSideSway, DesiredSideSway, SideSwaySpeed * DeltaTime);
	CurrentSideSway = SideSwayDelta;
	
	FVector FullSway = CurrentSideSway + CurrentForwardSway;
	
	CurrentLoc += FullSway;
}

void UWeaponComponent::HandleMouseSway(float DeltaTime)
{
	if(!WeaponActor)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}
	
	float PitchDirection = FMath::Clamp(MouseInput.Y, -1.f, 1.f);
	float DesiredPitch = MouseSwayAmount * PitchDirection;
	float PitchDelta = FMath::Lerp(CurrentPitch, DesiredPitch, MouseSwaySpeed * DeltaTime);
	CurrentPitch = PitchDelta;

	float YawDirection = FMath::Clamp(MouseInput.X, -1.f, 1.f);
	float DesiredYaw = MouseSwayAmount * -YawDirection;
	float YawDelta = FMath::Lerp(CurrentYaw, DesiredYaw, MouseSwaySpeed * DeltaTime);
	CurrentYaw = YawDelta;

	FRotator DeltaRot(CurrentPitch, CurrentYaw, 0);
	
	CurrentRot += DeltaRot;
}

void UWeaponComponent::HandleLandingImpact(float DeltaTime)
{
	if(!LandingBobCurve) return;
	
	const float Current = DeltaTime / LandingBobTime;
	LandingImpactTimer += Current;
	LandingImpactTimer = LandingImpactTimer > 1.f ? 1.f : LandingImpactTimer;
	
	CurrentLoc += FVector::UpVector * LandingBobCurve->GetFloatValue(LandingImpactTimer) * ImpactForce;
	
	if(LandingImpactTimer >= 1.0f)
	{
		bLandingImpact = false;
	}
}

void UWeaponComponent::StartLandingBob(float NewImpactForce)
{
	bLandingImpact = true;
	
	LandingImpactTimer = 0.0f;
	ImpactForce = NewImpactForce;
}
