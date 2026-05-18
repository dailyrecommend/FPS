#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponBase.h"
#include "Combat/Data/WeaponHitResult.h"
#include "Combat/Interfaces/Damageable.h"
#include "GunWeapon.generated.h"

class UAnimMontage;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGunFired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UGunWeapon : public UWeaponBase
{
    GENERATED_BODY()

public:
    UGunWeapon();

    UFUNCTION(BlueprintCallable, Category = "Gun")
    void FireChargedShot(float Damage, float FireLockoutSeconds);

    UFUNCTION(BlueprintCallable, Category = "Gun")
    void FireRicochetShot(float Damage, int32 RicochetCount, float FireLockoutSeconds);

    UPROPERTY(BlueprintAssignable, Category = "Gun") FOnGunFired OnGunFired;
    UPROPERTY(BlueprintAssignable, Category = "Gun") FOnGunHit   OnGunHit;

    virtual bool TryAttack_Implementation() override;

protected:
    void    FireHitscan(float Damage, EHitType HitType);
    AActor* FindNearestEnemy(const FVector& Origin, AActor* IgnoreActor) const;
    void    SpawnTrailEffect(const FVector& Start, const FVector& End);
    void    ApplyCooldownAfterShot(float FireLockoutSeconds);

    UPROPERTY(EditDefaultsOnly, Category = "Gun") float FireRange        = 10000.f;
    UPROPERTY(EditDefaultsOnly, Category = "Gun") float FireDamage       = 50.f;
    UPROPERTY(EditDefaultsOnly, Category = "Gun") float AutoTargetRadius = 800.f;


    virtual void EndPlay(EEndPlayReason::Type Reason) override;
    
    void ExecuteRicochetChain(int32 BounceIndex, int32 TotalBounces,
                                  FVector Origin, FVector Direction,
                                  float Damage, AActor* LastHitActor);

    UPROPERTY(EditDefaultsOnly, Category = "Gun")
    float RicochetInterval = 0.05f;

    TArray<FTimerHandle> RicochetTimers;

    
    UPROPERTY(EditDefaultsOnly, Category = "Gun|FX")
    FName MuzzlePointName = TEXT("MuzzlePoint");

    UPROPERTY(EditDefaultsOnly, Category = "Gun|Anim") TObjectPtr<UAnimMontage>   FireMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Gun|FX")   TObjectPtr<UNiagaraSystem> BulletTrailFX;
};