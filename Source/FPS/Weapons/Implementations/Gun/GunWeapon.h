#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponBase.h"
#include "Combat/Data/WeaponHitResult.h"
#include "GunWeapon.generated.h"

class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGunFired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UGunWeapon : public UWeaponBase
{
	GENERATED_BODY()

public:
	UGunWeapon();

	/**
	 * Public entry point for charged-shot skills (Focus). The skill component calls this
	 * after computing its own damage value and a fire lockout window.
	 * Going through a documented method (instead of friending FocusComponent) keeps the
	 * collaboration explicit and testable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gun")
	void FireChargedShot(float Damage, float FireLockoutSeconds);

	UPROPERTY(BlueprintAssignable, Category = "Gun") FOnGunFired OnGunFired;
	UPROPERTY(BlueprintAssignable, Category = "Gun") FOnGunHit   OnGunHit;

	virtual bool TryAttack_Implementation() override;

protected:
	void FireHitscan(float Damage, EHitType HitType);
	void PlayFireMontage();

	UPROPERTY(EditDefaultsOnly, Category = "Gun") float                  FireRange  = 10000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun") float                  FireDamage = 50.f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun") TObjectPtr<UAnimMontage> FireMontage;
};