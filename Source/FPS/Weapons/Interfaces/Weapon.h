#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Weapon.generated.h"

class IWeaponSkill;

UINTERFACE(MinimalAPI, BlueprintType)
class UWeapon : public UInterface
{
	GENERATED_BODY()
};

/**
 * Weapon abstraction. Concrete weapons (Gun, Sword, Bow, ...) implement this interface
 * via a component. PlayerCharacter routes input through IWeapon and never knows the
 * concrete type, so adding a weapon requires no edits to existing code.
 */
class FPS_API IWeapon
{
	GENERATED_BODY()

public:
	/** Attempt the weapon's primary action (fire / swing / etc). Returns true if it ran. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	bool TryAttack();

	/** Optional skill (Focus, Iajutsu, etc). Returns null when the weapon has none. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	TScriptInterface<IWeaponSkill> GetSkill() const;

	/** Called by the registry when this weapon becomes the active one. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void OnEquipped();

	/** Called by the registry when this weapon stops being the active one. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void OnUnequipped();

	/** Stable identifier — used by the registry for lookup and swap-by-id. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	FName GetWeaponId() const;

	/** Weapons that block movement abilities while active (e.g. heavy charge). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	bool BlocksMovement() const;
};