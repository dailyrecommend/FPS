#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponSkill.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UWeaponSkill : public UInterface
{
	GENERATED_BODY()
};

/**
 * Hold-style weapon skill. Used for "press and hold to charge, release to fire" patterns
 * such as Focus and Iajutsu. Single-shot skills go through IWeapon::TryAttack instead.
 */
class FPS_API IWeaponSkill
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Skill")
	void StartHold();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Skill")
	void EndHold();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Skill")
	bool IsActive() const;

	/** When true, movement and weapon-swap inputs should be ignored. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Skill")
	bool BlocksMovement() const;
};