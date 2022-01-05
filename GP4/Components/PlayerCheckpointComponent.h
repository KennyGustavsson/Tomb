#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCheckpointComponent.generated.h"

USTRUCT(BlueprintType)
struct FCheckpointData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FTransform CheckpointTransform;

	UPROPERTY(Transient)
	float LavaTimer = 0;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4_API UPlayerCheckpointComponent : public UActorComponent
{
	GENERATED_BODY()

	public:
	UPlayerCheckpointComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

// Functions
	UFUNCTION(BlueprintCallable)
	void SaveCheckpoint(FCheckpointData Checkpoint);
	UFUNCTION(BlueprintPure)
	FCheckpointData GetCheckpointData() const;
	UFUNCTION(BlueprintCallable)
	void MoveOwnerToCheckpoint() const;

// Variables
	UPROPERTY(EditDefaultsOnly)
	float ZKillHeight = -1000.0f;
	UPROPERTY(Transient)
	FCheckpointData CheckpointData;
	UPROPERTY(Transient)
	class ALava* Lava;
};