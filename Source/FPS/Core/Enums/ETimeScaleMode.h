#pragma once
#include "CoreMinimal.h"
#include "ETimeScaleMode.generated.h"

UENUM(BlueprintType)
enum class ETimeScaleMode : uint8
{
	/** Slow only the world; the requester stays at normal speed via CustomTimeDilation. */
	WorldOnly,

	/** Slow both the world and the requester. */
	Full,
};