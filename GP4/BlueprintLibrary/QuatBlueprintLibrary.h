#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuatBlueprintLibrary.generated.h"

UCLASS()
class GP4_API UQuatBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Rotation Slerp", CompactTitle = "Slerp", Category = "Custom"))
	static FRotator Slerp(FRotator const A, FRotator const B, float const Alpha);
};
