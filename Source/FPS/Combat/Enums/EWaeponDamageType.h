#pragma once
#include "CoreMinimal.h"
#include "EWeaponDamageType.generated.h"

UENUM(BlueprintType)
enum class EWeaponDamageType : uint8
{
	None,
	Gun,
	Sword,
	Iajutsu,
	Explosion,
	Environmental,
};