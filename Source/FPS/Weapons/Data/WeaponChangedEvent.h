#pragma once
#include "CoreMinimal.h"
#include "WeaponChangedEvent.generated.h"

USTRUCT(BlueprintType)
struct FPS_API FWeaponChangedEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FName PreviousWeaponId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FName NewWeaponId = NAME_None;
};