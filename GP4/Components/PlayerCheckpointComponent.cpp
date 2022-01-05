#include "PlayerCheckpointComponent.h"
#include "GP4/GP4MovementComponent.h"
#include "GP4/GP4Player.h"
#include "GP4/Lava/Lava.h"
#include "Kismet/GameplayStatics.h"

UPlayerCheckpointComponent::UPlayerCheckpointComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerCheckpointComponent::BeginPlay()
{
	Super::BeginPlay();

	FCheckpointData StartCheckpoint;
	StartCheckpoint.CheckpointTransform = GetOwner()->GetTransform();
	StartCheckpoint.LavaTimer = 0.0f;

	SaveCheckpoint(StartCheckpoint);

	TArray<AActor*> LavaActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALava::StaticClass(), LavaActors);

	if(LavaActors.Num() >= 1)
	{
		Lava = Cast<ALava>(LavaActors[0]);
	}
}

void UPlayerCheckpointComponent::TickComponent(float DeltaTime, ELevelTick Tick,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);

	if(GetOwner()->GetActorLocation().Z < ZKillHeight)
	{
		MoveOwnerToCheckpoint();
	}
}

void UPlayerCheckpointComponent::SaveCheckpoint(FCheckpointData Checkpoint)
{
	CheckpointData = Checkpoint;
}

FCheckpointData UPlayerCheckpointComponent::GetCheckpointData() const
{
	return CheckpointData;
}

void UPlayerCheckpointComponent::MoveOwnerToCheckpoint() const
{
	if(Lava)
	{
		Lava->SetLavaLevel(CheckpointData.LavaTimer);
	}

	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow, TEXT("Player Checkpoint teleport"));
	
	GetOwner()->SetActorLocation(CheckpointData.CheckpointTransform.GetLocation());
	GetOwner()->SetActorRotation(CheckpointData.CheckpointTransform.GetRotation());
	
	AGP4Player* Character = Cast<AGP4Player>(GetOwner());
	if(Character)
	{
		Character->SetActorLocation(CheckpointData.CheckpointTransform.GetLocation());
		
		Character->MovementComponent->Velocity = FVector::ZeroVector;
		Character->Controller->SetControlRotation(CheckpointData.CheckpointTransform.GetRotation().Rotator());
	}
}
