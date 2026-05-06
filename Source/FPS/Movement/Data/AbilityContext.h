#pragma once
#include "CoreMinimal.h"
#include "AbilityContext.generated.h"

USTRUCT(BlueprintType)
struct FPS_API FAbilityContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FVector2D MoveInput = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	bool bGrounded = false;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FVector Velocity = FVector::ZeroVector;
};