#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

USTRUCT(BlueprintType)
struct FOnInteractInfo
{
	GENERATED_BODY()
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, FOnInteractInfo, InteractInfo);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

// Functions
	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnInteract OnInteract;
};
