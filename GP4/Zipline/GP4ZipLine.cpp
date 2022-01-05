// Fill out your copyright notice in the Description page of Project Settings.


#include "GP4ZipLine.h"

#include "CableComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GP4/GP4MovementComponent.h"
#include "GP4/GP4Player.h"
#include "Kismet/GameplayStatics.h"

AGP4ZipLine::AGP4ZipLine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	RootComponent = RootScene;
	
	SplineZipperLine = CreateDefaultSubobject<USplineComponent>(TEXT("Spline Zipper Line"));
	SplineZipperLine->SetupAttachment(RootScene);
	PipeMesh = CreateDefaultSubobject<UStaticMesh>(TEXT("Static Pipe Mesh"));
	
	StartCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Start Collider"));
	StartCollider->SetupAttachment(RootScene);
}

// Called when the game starts or when spawned
void AGP4ZipLine::BeginPlay()
{
	Super::BeginPlay();
	ZipperLength = SplineZipperLine->GetSplineLength();
	// Will Change that into On Overlap Get Actor , For now Will get Pawn and cast to player.
	Player = Cast<AGP4Player>( UGameplayStatics::GetPlayerPawn(GetWorld(),0));
	if(Player)
	{
		Player->OnJumpEvent.AddDynamic(this, &AGP4ZipLine::OnPlayerJump);		// set up a notification for when this component hits something blocking
	}
}

// Called every frame
void AGP4ZipLine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsSliding)
	{
		TempDistance += ZipperLength * DeltaTime * SlidingSpeed;
		const FVector NewPlayerLocation = SplineZipperLine->GetLocationAtDistanceAlongSpline(
			TempDistance, ESplineCoordinateSpace::World);
		Player->SetActorLocation(NewPlayerLocation + PlayerOffset);
		if (TempDistance >= ZipperLength)
		{
			bIsSliding = false;
			Player->MovementComponent->bIsPauseToLeap = false;
		}
	}
}

void AGP4ZipLine::OnPlayerJump()
{
	if(bIsSliding)
	{
		bIsSliding = false;
		TempDistance = 0;
		Player->MovementComponent->bIsPauseToLeap = false;
		Player->MovementComponent->RopeJump(GravityGraceTimeJump, ReleaseJumpUpwardForce, Player->GetActorForwardVector() * ReleaseJumpForwardForce);
	}
}

/* Designer tool to create Zipper line in editor. Taking Spline Points And adding Zipper Mesh from Point to another */

void AGP4ZipLine::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	for (int32 i = 0; i < SplineZipperLine->GetNumberOfSplinePoints() -1; ++i)
	{
		USplineMeshComponent* StaticMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass(), TEXT("Zipper Mesh point: " + i));
		
		FVector StartLocation,StartTangent, EndLocation,EndTangent;

		SplineZipperLine->GetLocationAndTangentAtSplinePoint(i,StartLocation,StartTangent,ESplineCoordinateSpace::World);
		SplineZipperLine->GetLocationAndTangentAtSplinePoint(i + 1,EndLocation,EndTangent,ESplineCoordinateSpace::World);

		StaticMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation,EndTangent,true);

		StaticMeshComponent->SetMobility(EComponentMobility::Type::Movable);
		StaticMeshComponent->SetForwardAxis(ESplineMeshAxis::Y);
		StaticMeshComponent->SetStaticMesh(PipeMesh);
		
		StaticMeshComponent->RegisterComponent();
		StaticMeshComponent->UpdateMesh();
	}
	
	SplineZipperLine->UpdateSpline();
}

void AGP4ZipLine::StartSliding()
{
	TempDistance = 0;
	Player->MovementComponent->bIsPauseToLeap = true;
	bIsSliding = true;
}
