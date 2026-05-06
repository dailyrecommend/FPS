#pragma once
#include "CoreMinimal.h"
#include "EHitType.generated.h"

UENUM(BlueprintType)
enum class EHitType : uint8
{
	Normal,
	Critical,
	Headshot,
	Charged,
	Parried,
	Blocked,
};