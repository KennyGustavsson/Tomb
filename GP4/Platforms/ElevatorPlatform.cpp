#include "ElevatorPlatform.h"

AElevatorPlatform::AElevatorPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	ElevatorMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ElevatorMeshComponent->SetupAttachment(Root);
	ElevatorMeshComponent->SetEnableGravity(false);
}

void AElevatorPlatform::BeginPlay()
{
	Super::BeginPlay();

	ElevatorPrimitiveComponent = ElevatorMeshComponent;
}

void AElevatorPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ZVelocity += -ZGravity * DeltaTime;
	ElevatorPrimitiveComponent->AddRelativeLocation(FVector(0,0, ZVelocity));

	if(ElevatorPrimitiveComponent->GetComponentLocation().Z < MinZHeight)
	{
		ZVelocity = 0.0f;
		
		ElevatorPrimitiveComponent->SetWorldLocation(FVector(
			ElevatorPrimitiveComponent->GetComponentLocation().X,
			ElevatorPrimitiveComponent->GetComponentLocation().Y,
			MinZHeight));

		PrimaryActorTick.SetTickFunctionEnable(false);
	}

	if(ElevatorPrimitiveComponent->GetComponentLocation().Z > MaxZHeight)
	{
		ElevatorPrimitiveComponent->SetWorldLocation(FVector(
			ElevatorPrimitiveComponent->GetComponentLocation().X,
			ElevatorPrimitiveComponent->GetComponentLocation().Y,
			MaxZHeight));
	}
}

void AElevatorPlatform::AddForceToElevator()
{
	PrimaryActorTick.SetTickFunctionEnable(true);
	
	ZVelocity = 0.0f;
	ZVelocity += Force;
}
