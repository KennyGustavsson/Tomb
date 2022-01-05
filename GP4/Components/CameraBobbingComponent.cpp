#include "CameraBobbingComponent.h"
#include "Camera/CameraComponent.h"
#include "GP4/GP4Player.h"

UCameraBobbingComponent::UCameraBobbingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraBobbingComponent::BeginPlay()
{
	Super::BeginPlay();

	AGP4Player* Player = Cast<AGP4Player>(GetOwner());

	if(Player)
	{
		CameraComponent = Player->CameraComp;
		CameraOriginalRelativeLocation = CameraComponent->GetRelativeLocation();
	}
}

void UCameraBobbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!CameraComponent)
	{
		return;
	}
	
	if(bHeadBob)
	{
		HeadBobLerp(DeltaTime);
	}
	
	if(bLandingImpact)
	{
		LandingImpactLerp(DeltaTime);
	}

	if(bJumpBob)
	{
		JumpingBobLerp(DeltaTime);
	}

	if(bHeadBob || bLandingImpact || bJumpBob)
	{
		SetRelativeLocationBlend();
	}
}

void UCameraBobbingComponent::HeadBob()
{
	bHeadBob = true;
	
	if(HeadBobAccumulator >= 1.0f)
	{
		HeadBobAccumulator = 0.0f;
	}
}

void UCameraBobbingComponent::LandingImpact(float ImpactForce)
{
	bLandingImpact = true;
	
	if(LandingImpactAccumulator >= 1.0f)
	{
		LandingImpactAccumulator = 0.0f;
		CurrentImpactForce = ImpactForce;
	}
}

void UCameraBobbingComponent::JumpingBob()
{
	bJumpBob = true;

	if(JumpBobAccumulator >= 1.0f)
	{
		JumpBobAccumulator = 0.0f;
	}
}

void UCameraBobbingComponent::HeadBobLerp(float DeltaTime)
{
	if(!HeadBobCurve) return;
	
	const float Current = DeltaTime / HeadBobTime;
	HeadBobAccumulator += Current;
	HeadBobAccumulator = HeadBobAccumulator > 1.f ? 1.f : HeadBobAccumulator;
	
	CurrentZ += HeadBobCurve->GetFloatValue(HeadBobAccumulator);
	
	if(HeadBobAccumulator >= 1.0f)
	{
		bHeadBob = false;
	}
}

void UCameraBobbingComponent::LandingImpactLerp(float DeltaTime)
{
	if(!LandingImpactCurve) return;
	
	const float Current = DeltaTime / LandingImpactTime;
	LandingImpactAccumulator += Current;
	LandingImpactAccumulator = LandingImpactAccumulator > 1.f ? 1.f : LandingImpactAccumulator;
	
	CurrentZ += LandingImpactCurve->GetFloatValue(LandingImpactAccumulator) * CurrentImpactForce;
	
	if(LandingImpactAccumulator >= 1.0f)
	{
		bLandingImpact = false;
	}
}

void UCameraBobbingComponent::JumpingBobLerp(float DeltaTime)
{
	if(!JumpBobCurve) return;
	
	const float Current = DeltaTime / JumpBobTime;
	JumpBobAccumulator += Current;
	JumpBobAccumulator = JumpBobAccumulator > 1.f ? 1.f : JumpBobAccumulator;
	
	CurrentZ += JumpBobCurve->GetFloatValue(JumpBobAccumulator);
	
	if(JumpBobAccumulator >= 1.0f)
	{
		bJumpBob = false;
	}
}

void UCameraBobbingComponent::SetRelativeLocationBlend()
{
	CameraComponent->SetRelativeLocation(FVector(0, 0, CurrentZ + CameraOriginalRelativeLocation.Z));
	CurrentZ = 0.0f;
}

