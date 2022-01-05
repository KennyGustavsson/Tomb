#include "QuatBlueprintLibrary.h"

FRotator UQuatBlueprintLibrary::Slerp(FRotator const A, FRotator const B, float const Alpha)
{
	return FQuat::Slerp(A.Quaternion(), B.Quaternion(), Alpha).Rotator();
}
