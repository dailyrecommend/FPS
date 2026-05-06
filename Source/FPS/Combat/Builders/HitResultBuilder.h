#pragma once
#include "CoreMinimal.h"
#include "Combat/Data/WeaponHitResult.h"

/**
 * Fluent builder for FWeaponHitResult.
 *
 * Used by every weapon to assemble hit data, replacing the duplicated boilerplate that
 * lived in GunComponent, SwordComponent, IajutsuComponent in the original codebase.
 *
 * Lightweight C++ class (not a UObject) — meant to be stack-allocated and chained.
 *
 * Usage:
 *   FHitResultBuilder()
 *       .From(GetController())
 *       .FromHit(EngineHit)
 *       .WithDamage(25.f)
 *       .OfDamageType(EWeaponDamageType::Gun)
 *       .Apply();
 */
class FPS_API FHitResultBuilder
{
public:
	FHitResultBuilder() = default;

	FHitResultBuilder& From(AController* InInstigator);
	FHitResultBuilder& Target(AActor* Actor);
	FHitResultBuilder& At(const FVector& Loc, const FVector& Normal);
	FHitResultBuilder& WithDamage(float Damage);
	FHitResultBuilder& OfDamageType(EWeaponDamageType Type);
	FHitResultBuilder& OfHitType(EHitType Type);
	FHitResultBuilder& AsCritical(bool bCrit = true);
	FHitResultBuilder& FromHit(const FHitResult& EngineHit);

	/** Returns a copy of the assembled result without applying. */
	FWeaponHitResult Build() const;

	/**
	 * Build and dispatch to the target if it implements IDamageable.
	 * Returns true if dispatched, false if no valid target / not damageable.
	 * Never crashes on null inputs.
	 */
	bool Apply() const;

private:
	FWeaponHitResult Result;
};