#pragma once
#include "CoreMinimal.h"
#include "EActivationResult.generated.h"

UENUM(BlueprintType)
enum class EActivationResult : uint8
{
	Success,
	Failed_OnCooldown,
	Failed_NotReady,
	Failed_Blocked,
	Failed_NoOwner,
};