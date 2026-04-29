#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/WeaponHitResult.h"
#include "GunComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGunFired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunComponent();

	void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera);

	void TryFire();

	// Called by FocusComponent on focus release
	void PerformChargedShot(float Damage, float FireLockout);

	UPROPERTY(BlueprintAssignable) FOnGunFired OnGunFired;
	UPROPERTY(BlueprintAssignable) FOnGunHit   OnHit;

	UFUNCTION(BlueprintPure) bool CanFire() const;

protected:
	virtual void BeginPlay() override;

private:
	void PerformHitscan(float Damage, EHitType HitType);
	void PlayFireMontage();

	UPROPERTY(EditDefaultsOnly, Category = "Gun") float         FireRange    = 10000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun") float         FireDamage   = 50.f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun") float         FireCooldown = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Gun") UAnimMontage* FireMontage;

	UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
	UPROPERTY() UCameraComponent* Camera         = nullptr;

	float LastFireTime = 0.f;
};