#include "Lava.h"
#include "GP4/Components/PlayerCheckpointComponent.h"

ALava::ALava()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALava::BeginPlay()
{
	Super::BeginPlay();

	Player = GetWorld()->GetFirstPlayerController()->GetPawn();

	if(!LavaCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("No float curve on lava actor"));
	}
}

void ALava::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(!LavaCurve)
	{
		return;
	}
	
	ZHeight = LavaCurve->GetFloatValue(Timer);
	SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, ZHeight));
	
	if(Player)
	{
		if(Player->GetActorLocation().Z < ZHeight)
		{
			TArray<UPlayerCheckpointComponent*> CheckpointComponents;
			Player->GetComponents(CheckpointComponents);

			for(const UPlayerCheckpointComponent* CheckpointComponent : CheckpointComponents)
			{
				CheckpointComponent->MoveOwnerToCheckpoint();
			}
		}
	}
	
	Timer += DeltaSeconds * bEnabled;
}

void ALava::SetLavaLevel(float LavaTime)
{
	Timer = LavaTime;
	ZHeight = LavaCurve->GetFloatValue(Timer);
	SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, ZHeight));
}

void ALava::SetEnabled(bool Enabled)
{
	bEnabled = Enabled;
}
