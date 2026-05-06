#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CameraEffects.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCameraEffects : public UInterface
{
	GENERATED_BODY()
};

/**
 * Camera effects abstraction.
 *
 * Effects are managed by handle so multiple systems (Focus, Glissando, hit-stop, ...)
 * can request changes simultaneously without overwriting each other. The component
 * decides the resulting state (typically: highest-priority active request wins).
 *
 * All Push methods return a handle. 0 = rejected. Bad handles in Pop are ignored.
 */
class FPS_API ICameraEffects
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	int32 PushFOVOffset(float Offset, float InterpSpeed, int32 Priority);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	void PopFOVOffset(int32 Handle);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	int32 PushRollOffset(float RollDegrees, float InterpSpeed, int32 Priority);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	void PopRollOffset(int32 Handle);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	int32 PushHeightOffset(float HeightOffset, float InterpSpeed, int32 Priority);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	void PopHeightOffset(int32 Handle);

	/** Update the live value of an existing handle (for continuously-changing effects like Glissando roll). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	bool UpdateRollOffset(int32 Handle, float NewRollDegrees);
};