#pragma once
#include "CoreMinimal.h"
#include "Core/Enums/ETimeScaleMode.h"
#include "TimeDilationRequest.generated.h"

USTRUCT(BlueprintType)
struct FPS_API FTimeDilationRequest
{
	GENERATED_BODY()

	/** Target world dilation. Must be > 0. Values <= 0 are rejected on Push. */
	UPROPERTY(BlueprintReadWrite, Category = "TimeDilation")
	float WorldDilation = 0.2f;

	UPROPERTY(BlueprintReadWrite, Category = "TimeDilation")
	ETimeScaleMode Mode = ETimeScaleMode::WorldOnly;

	/** Blend duration when starting the request (in seconds). 0 = instant. */
	UPROPERTY(BlueprintReadWrite, Category = "TimeDilation")
	float BlendIn = 0.1f;

	/** Blend duration when releasing the request (in seconds). 0 = instant. */
	UPROPERTY(BlueprintReadWrite, Category = "TimeDilation")
	float BlendOut = 0.2f;

	/** Higher priority wins. Ties broken by stronger slowdown (smaller dilation). */
	UPROPERTY(BlueprintReadWrite, Category = "TimeDilation")
	int32 Priority = 0;

	/** The actor making the request. Tracked weakly so we can auto-clean on destruction. */
	UPROPERTY(BlueprintReadWrite, Category = "TimeDilation")
	TObjectPtr<AActor> Requester = nullptr;
};