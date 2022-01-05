#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckpointTrigger.generated.h"

UCLASS()
class GP4_API ACheckpointTrigger : public AActor
{
	GENERATED_BODY()

public:
	ACheckpointTrigger();
	virtual void BeginPlay() override;
	
// Components
	UPROPERTY(EditAnywhere)
	class USceneComponent* Root;
	UPROPERTY(EditAnywhere)
	class UBoxComponent* Box;
	UPROPERTY(EditAnywhere)
	class UArrowComponent* CheckpointSpawn;

// Function
	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintImplementableEvent)
	void OnCheckpointReached();

// Variables
	UPROPERTY(EditAnywhere)
	float LavaMinTime = 0.0f;
	UPROPERTY(EditDefaultsOnly)
	bool bDebug = false;
	UPROPERTY(Transient)
	class ALava* Lava;
};
