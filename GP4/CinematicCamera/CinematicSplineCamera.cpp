#include "CinematicSplineCamera.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetMathLibrary.h"

ACinematicSplineCamera::ACinematicSplineCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
}

void ACinematicSplineCamera::BeginPlay()
{
	Super::BeginPlay();

	auto const Controller = GetWorld()->GetFirstPlayerController();
	Controller->SetViewTargetWithBlend(this, 0.0f);

	for (auto const Actor : SplineActors)
	{
		TArray<USplineComponent*> SplineComp;
		Actor->GetComponents(SplineComp);

		if(SplineComp.Num() > 0)
		{
			Splines.Add(SplineComp[0]);
		}
	}

	while (Speed.Num() < Splines.Num())
	{
		Speed.Add(DefaultSpeed);
	}

	while (LookAtActors.Num() < Splines.Num())
	{
		LookAtActors.Add(nullptr);
	}

	SetActorLocation(Splines[Index]->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World));
}

void ACinematicSplineCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(Index >= Splines.Num())
	{
		return;
	}

	float const InputKey = Splines[Index]->FindInputKeyClosestToWorldLocation(GetActorLocation());
	float Distance = Splines[Index]->GetDistanceAlongSplineAtSplineInputKey(InputKey);
	Distance += KeyDistance;
	
	FVector const Loc = Splines[Index]->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	FVector const TargetDirection = (Loc - GetActorLocation()).GetSafeNormal();
	
	AddActorWorldOffset(TargetDirection * Speed[Index] * DeltaTime);

	if(LookAtActors[Index])
	{
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookAtActors[Index]->GetActorLocation()));
	}
	
	if(Splines[Index]->GetSplineLength() <= Splines[Index]->GetDistanceAlongSplineAtSplineInputKey(InputKey))
	{
		Index++;
		OnNextIndex();
	}
	else if(!LookAtActors[Index])
	{
		SetActorRotation(TargetDirection.Rotation());
	}
}

void ACinematicSplineCamera::OnNextIndex()
{
	PrimaryActorTick.SetTickFunctionEnable(false);

	CurrentEnd.Broadcast();

	if(Index < Splines.Num())
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACinematicSplineCamera::SetTickActive, TimeBetweenSplines, false);
	}
}

void ACinematicSplineCamera::SetTickActive()
{
	OnNext.Broadcast();
	
	SetActorLocation(Splines[Index]->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World));
	PrimaryActorTick.SetTickFunctionEnable(true);
}

