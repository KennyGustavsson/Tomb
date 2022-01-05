#include "Platform.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"

APlatform::APlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Box);
	
	PlatformSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	PlatformSpline->SetupAttachment(Root);
}

void APlatform::BeginPlay()
{
	Super::BeginPlay();

	OriginalRotation = Box->GetRelativeRotation().Quaternion();
	
	UpdateMoveTowardsPointOnSpline();
	SetActive(bActive);
}

void APlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(StateSequence.Num() <= 0) return;

	if(Easing == EEasing::CustomCurve && !SplineSpeedMultiplierCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("No custom curve is selected"));
		return;
	}

	if(bForceReset)
	{
		ForceReset(DeltaTime);
		return;
	}
	
	switch (StateSequence[CurrentState])
	{
		case EPlatformState::Moving:
			Moving(DeltaTime);
			break;
		
		case EPlatformState::Waiting:
			Waiting(DeltaTime);
			break;
		
		case EPlatformState::Rotating:
			Rotating(DeltaTime);
			break;

		case EPlatformState::MoveTowardsPointOnSpline:
			MoveTowardsPointOnSpline(DeltaTime);
			break;

		case EPlatformState::ResetToZero:
			ResetToZero(DeltaTime);
			break;

		default:
			;
	}
}

void APlatform::NextState()
{
	if(StateSequence.Num() <= 0) return;
	
	StartStateAccumulator = Accumulator;
	
	switch (StateSequence[CurrentState])
	{
		case EPlatformState::Moving:
			if(!PlatformSpline->IsClosedLoop())
			{
				Direction *= -1;
			}
			break;
			
		case EPlatformState::Waiting:
			WaitingAccumulator = 0.0f;
			break;
			
		case EPlatformState::Rotating:
			RotationAccumulator = 0.0f;
			break;

		case EPlatformState::MoveTowardsPointOnSpline:
			break;

		case EPlatformState::ResetToZero:
			Direction = 1;
			WaitingAccumulator = 0.0f;
			RotationAccumulator = 0.0f;
			CurrentActor = 0;
			break;

		default:
			;
	}

	OnStateFinnish.Broadcast();
	
	if(bDoOnce)
	{
		CurrentState++;

		if(CurrentState >= StateSequence.Num())
		{
			CurrentState = 0;
			SetActive(false);
		}
	}
	else
	{
		CurrentState = (CurrentState + 1) % StateSequence.Num();
	}
	
	if(bSetDisableOnStateChange)
	{
		bActive = false;
		SetActive(bActive);
	}
	
	switch (StateSequence[CurrentState])
	{
		case EPlatformState::Moving:
			break;
			
		case EPlatformState::Waiting:
			break;
			
		case EPlatformState::Rotating:
			StartRot = Box->GetRelativeRotation().Quaternion();
			EndRot = (Box->GetRelativeRotation() + Rotation).Quaternion();
			break;

		case EPlatformState::MoveTowardsPointOnSpline:
			CurrentActor = (CurrentActor + 1) % MoveTowardsActorOnSpline.Num();
			UpdateMoveTowardsPointOnSpline();
		
		case EPlatformState::ResetToZero:
			StartRot = Box->GetRelativeRotation().Quaternion();
			EndRot = OriginalRotation;
			break;
		
		default:
			;
	}
}

void APlatform::SetActive(bool const Active)
{
	PrimaryActorTick.SetTickFunctionEnable(Active);
	bActive = Active;
	OnActivate.Broadcast();
}

void APlatform::UpdateMoveTowardsPointOnSpline()
{
	if(MoveTowardsActorOnSpline.Num() > 0 && MoveTowardsActorOnSpline.Num() > CurrentActor)
	{
		if(MoveTowardsActorOnSpline[CurrentActor])
		{
			MoveTowardsActorOnSplineKey = PlatformSpline->FindInputKeyClosestToWorldLocation(MoveTowardsActorOnSpline[CurrentActor]->GetActorLocation());
			MoveTowardsActorOnSplineKey = MoveTowardsActorOnSplineKey < 0.f ? 0.f : MoveTowardsActorOnSplineKey;
			MoveTowardsActorOnSplineKey = MoveTowardsActorOnSplineKey > 1.f ? 1.f : MoveTowardsActorOnSplineKey;
			Direction = MoveTowardsActorOnSplineKey < Accumulator ? -1 : 1;
		}
	}
}

void APlatform::ForceReset(bool AfterResetActive)
{
	bDeactivateAfterReset = AfterResetActive;
	bForceReset = true;
	bWasActive = IsActive();
}

void APlatform::Moving(float const DeltaTime)
{
	const float Current = DeltaTime / SplineTravelTime;
	Accumulator += Current * Direction;
	
	Accumulator = Accumulator < 0.f ? 0.f : Accumulator;
	Accumulator = Accumulator > 1.f ? 1.f : Accumulator;

	switch (Easing)
	{
	case EEasing::NoEasing:
		Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(Accumulator, ESplineCoordinateSpace::World));
		break;

	case EEasing::InOutSine:
		Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(EaseInOutSine(Accumulator), ESplineCoordinateSpace::World));
		break;

	case EEasing::CustomCurve:
		Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(SplineSpeedMultiplierCurve->GetFloatValue(Accumulator), ESplineCoordinateSpace::World));
		break;
		
	default:
		;
	}

	if(Accumulator <= 0.0f || Accumulator >= 1.0f)
	{
		NextState();
	}
}

void APlatform::Waiting(float const DeltaTime)
{
	WaitingAccumulator += DeltaTime;
	
	if(WaitingAccumulator >= WaitingTime)
	{
		NextState();
	}
}

void APlatform::Rotating(float const DeltaTime)
{
	const float Current = DeltaTime / RotationTime;
	RotationAccumulator += Current;

	RotationAccumulator = RotationAccumulator < 0.f ? 0.f : RotationAccumulator;
	RotationAccumulator = RotationAccumulator > 1.f ? 1.f : RotationAccumulator;

	switch (Easing)
	{
	case EEasing::NoEasing:
		Box->SetRelativeRotation(FQuat::Slerp(StartRot, EndRot, RotationAccumulator));
		break;

	case EEasing::InOutSine:
		Box->SetRelativeRotation(FQuat::Slerp(StartRot, EndRot, EaseInOutSine(RotationAccumulator)));
		break;

	case EEasing::CustomCurve:
		Box->SetRelativeRotation(FQuat::Slerp(StartRot, EndRot, SplineSpeedMultiplierCurve->GetFloatValue(RotationAccumulator)));
		break;
		
	default:
		;
	}

	if(RotationAccumulator <= 0.0f || RotationAccumulator >= 1.f)
	{
		NextState();
	}
}

void APlatform::MoveTowardsPointOnSpline(float const DeltaTime)
{
	const float Current = DeltaTime / SplineTravelTime;
	Accumulator += Current * Direction;
	
	Accumulator = Accumulator < 0.f ? 0.f : Accumulator;
	Accumulator = Accumulator > 1.0f ? 1.0f : Accumulator;

	switch (Easing)
	{
		case EEasing::NoEasing:
			Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(RemapStartStateAccumulator(MoveTowardsActorOnSplineKey, Accumulator), ESplineCoordinateSpace::World));
			break;

		case EEasing::InOutSine:
			Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(RemapWithEase(MoveTowardsActorOnSplineKey, Accumulator), ESplineCoordinateSpace::World));
			break;

		case EEasing::CustomCurve:
			Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(RemapWithCurve(MoveTowardsActorOnSplineKey, Accumulator), ESplineCoordinateSpace::World));
			break;
		
		default:
			;
	}

	if(Direction > 0)
	{
		if(Accumulator <= 0.0f || Accumulator >= MoveTowardsActorOnSplineKey)
		{
			NextState();
		}
	}
	else
	{
		if(Accumulator <= MoveTowardsActorOnSplineKey || Accumulator >= 1.0f)
		{
			NextState();
		}
	}
}

void APlatform::ResetToZero(float const DeltaTime)
{
	const float Current = DeltaTime / SplineTravelTime;
	Accumulator -= Current;

	const float CurrentRot = DeltaTime / RotationTime;
	RotationAccumulator += CurrentRot;
	
	Accumulator = Accumulator < 0.0f ? 0.0f : Accumulator;
	Accumulator = Accumulator > 1.0f ? 1.0f : Accumulator;

	RotationAccumulator = RotationAccumulator < 0.f ? 0.f : RotationAccumulator;
	RotationAccumulator = RotationAccumulator > 1.f ? 1.f : RotationAccumulator;
	
	if(SplineSpeedMultiplierCurve)
	{
		Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(SplineSpeedMultiplierCurve->GetFloatValue(Accumulator), ESplineCoordinateSpace::World));
	}
	else
	{
		Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(Accumulator, ESplineCoordinateSpace::World));
	}

	Box->SetRelativeRotation(FQuat::Slerp(StartRot, EndRot, RotationAccumulator));
	
	if((Accumulator <= 0.0f && RotationAccumulator <= 1.0f))
	{
		NextState();
	}
}

void APlatform::ForceReset(float const DeltaTime)
{
	const float Current = DeltaTime / SplineTravelTime;
	Accumulator -= Current;

	const float CurrentRot = DeltaTime / RotationTime;
	RotationAccumulator += CurrentRot;
	
	Accumulator = Accumulator < 0.0f ? 0.0f : Accumulator;
	Accumulator = Accumulator > 1.0f ? 1.0f : Accumulator;

	RotationAccumulator = RotationAccumulator < 0.f ? 0.f : RotationAccumulator;
	RotationAccumulator = RotationAccumulator > 1.f ? 1.f : RotationAccumulator;
	
	if(SplineSpeedMultiplierCurve)
	{
		Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(SplineSpeedMultiplierCurve->GetFloatValue(Accumulator), ESplineCoordinateSpace::World));
	}
	else
	{
		Box->SetWorldLocation(PlatformSpline->GetLocationAtTime(Accumulator, ESplineCoordinateSpace::World));
	}

	Box->SetRelativeRotation(FQuat::Slerp(StartRot, EndRot, RotationAccumulator));
	
	if((Accumulator <= 0.0f && RotationAccumulator <= 1.0f))
	{
		bForceReset = false;
		SetActive(bWasActive);

		if(bDeactivateAfterReset)
		{
			SetActive(bDeactivateAfterReset);
		}
	}
}

float APlatform::EaseInOutSine(float const X)
{
	return -(FMath::Cos(PI * X) - 1) * 0.5f;
}

float APlatform::InverseLerp(float const A, float const B, float const Distance)
{
	return FMath::Clamp((Distance - A) / (B - A), 0.0f, 1.0f);
}

float APlatform::RemapStartStateAccumulator(float const End, float const T) const
{
	return FMath::Lerp(StartStateAccumulator, End, InverseLerp(StartStateAccumulator, End, T));
}

float APlatform::RemapWithEase(float const End, float const T) const
{
	return FMath::Lerp(StartStateAccumulator, End, EaseInOutSine(InverseLerp(StartStateAccumulator, End, T)));
}

float APlatform::RemapWithCurve(float const End, float const T) const
{
	return FMath::Lerp(StartStateAccumulator, End, SplineSpeedMultiplierCurve->GetFloatValue(InverseLerp(StartStateAccumulator, End, T)));
}