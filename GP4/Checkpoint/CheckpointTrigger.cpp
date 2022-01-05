#include "CheckpointTrigger.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "GP4/Components/PlayerCheckpointComponent.h"
#include "GP4/Lava/Lava.h"
#include "Kismet/GameplayStatics.h"

ACheckpointTrigger::ACheckpointTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTrigger"));
	Box->SetupAttachment(Root);
	Box->SetCollisionProfileName("OverlapAllDynamic");

	CheckpointSpawn = CreateDefaultSubobject<UArrowComponent>(TEXT("Checkpoint Spawn"));
	CheckpointSpawn->SetupAttachment(Box);
}

void ACheckpointTrigger::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &ACheckpointTrigger::OnBeginOverlap);

	TArray<AActor*> LavaActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALava::StaticClass(), LavaActors);

	if(LavaActors.Num() >= 1)
	{
		Lava = Cast<ALava>(LavaActors[0]);
	}
}

void ACheckpointTrigger::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!OtherActor)
	{
		return;
	}
	
	TArray<UPlayerCheckpointComponent*> CheckpointComponents;
	OtherActor->GetComponents(CheckpointComponents);

	for (UPlayerCheckpointComponent* CheckpointComponent : CheckpointComponents)
	{
		if(bDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Checkpoint Reached"));
		}

		FCheckpointData CheckpointData;
		CheckpointData.CheckpointTransform = CheckpointSpawn->GetComponentTransform();

		if(Lava)
		{
			if(Lava->Timer > LavaMinTime)
			{
				CheckpointData.LavaTimer = LavaMinTime;
			}
			else
			{
				CheckpointData.LavaTimer = Lava->Timer;
			}
		}
		else
		{
			CheckpointData.LavaTimer = LavaMinTime;
		}
		
		CheckpointComponent->SaveCheckpoint(CheckpointData);
	}
}
