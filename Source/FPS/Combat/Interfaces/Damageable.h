#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Combat/Data/WeaponHitResult.h"
#include "Damageable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Anything that can be damaged by a weapon implements this.
 * Weapons never call functions on concrete enemy types — they go through this interface.
 */
class FPS_API IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void OnWeaponHit(const FWeaponHitResult& Hit);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsAlive() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	float GetCurrentHealth() const;
};