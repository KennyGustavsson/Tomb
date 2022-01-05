// Fill out your copyright notice in the Description page of Project Settings.


#include "GP4Rope.h"
#include "CableComponent.h"
#include "Components/BoxComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Platforms/Platform.h"

AGP4Rope::AGP4Rope()
{
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	RootComponent = RootComp;
	
	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable Component"));
	Cable->SetupAttachment(RootComp);

	EndCable = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndCable"));
	EndCable->SetupAttachment(RootComp);

	PhysicsConstraintComponent = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Physics Consttraint"));
	PhysicsConstraintComponent->SetupAttachment(RootComp);
	
	VisualGrappling = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Grappling Pt"));
	VisualGrappling->SetupAttachment(RootComp);
}

void AGP4Rope::BeginPlay()
{
	Super::BeginPlay();
	/*
	 * If bIsConnectedToPlatform Than we will attach the rop to the platform and update its position
	 * relative to the moving platform. 
	*/ 
	if(bIsConnectedToPlatform)
	{
		TArray<AActor*> Result;
		GetOverlappingActors(Result,APlatform::StaticClass());
		if(Result.Num()>0)
		{
			Platform = Cast<APlatform>(Result[0]);
			if(Platform == nullptr) return;
			InitialPlatformBoxOffsetLocation =  Platform->Box->GetComponentLocation() - GetActorLocation();
			GetWorld()->GetTimerManager().SetTimer(PositionUpdateTimer, this, &AGP4Rope::FollowPlatform, RefreshRateFollowPlatform, true);
		}
	}

	
	// UE4 Engine Problem, Turning On/Off Simulation as workaround to fix the issue.
	EndCable->SetSimulatePhysics(true);
	EndCable->SetSimulatePhysics(false);
}


/*Turn On Collision After Very short time , So player can not abuse Fast pressing hooks ( 0.3s is an ok value to prevent that ) */
void AGP4Rope::TurnOnCollision() const
{
	VisualGrappling->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AGP4Rope::FollowPlatform()
{
	const FVector NewPositionDifference = Platform->Box->GetComponentLocation() - (GetActorLocation() + InitialPlatformBoxOffsetLocation) ;
	SetActorLocation(GetActorLocation() + NewPositionDifference);
}

/* Hide rope and return it to it's Original State . Cable Length 0 , Turn off Sphere Physics and restore its original Components Transform*/ 
void AGP4Rope::ResetRope()
{
	
	Cable->CableLength = 0;
	Cable->SetVisibility(false);
	EndCable->SetSimulatePhysics(false);
	SetActorRotation(InitialRotation);
	PhysicsConstraintComponent->SetRelativeLocation(FVector(0,0,PhysicsConstraintComponent->GetRelativeLocation().Z));
	EndCable->SetWorldLocation(RootComp->GetComponentLocation());
	VisualGrappling->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorld()->GetTimerManager().SetTimer(CollisionTimer, this, &AGP4Rope::TurnOnCollision, HookRestPeriod, true);
}


/* Rope Length and End Cable Location are calculated in GrapplingComp based on Player Distance.
 * EndCableLocation will be in front of the player , So he can grab to "Turn physics On Cable End" .
 * Attaching Player Capsule to the Grappling Sphere will be done in GrapplingComp
 */
void AGP4Rope::UpdateRope(float CableLengthArg, FVector EndCableLocation, bool bIsSimulatePhysics) const
{
	Cable->CableLength = CableLengthArg - SubCableLength;
	EndCable->SetWorldLocation(EndCableLocation);
	PhysicsConstraintComponent->SetConstrainedComponents(nullptr,"",EndCable,"");
	Cable->SetVisibility(true);
	EndCable->SetSimulatePhysics(bIsSimulatePhysics);
}



