#pragma once
#include "CoreMinimal.h"
#include "CameraEffectRequest.generated.h"

USTRUCT()
struct FPS_API FCameraEffectRequest
{
	GENERATED_BODY()

	int32 Handle      = 0;
	float Value       = 0.f;
	float InterpSpeed = 8.f;
	int32 Priority    = 0;
};