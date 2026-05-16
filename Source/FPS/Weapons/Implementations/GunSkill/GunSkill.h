#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponSkillBase.h"
#include "GunSkill.generated.h"

class UAnimMontage;
class UGunWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunSkillStateChanged, bool, bIsCharging);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunSkillCharged, int32, RicochetCount);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UGunSkill : public UWeaponSkillBase
{
    GENERATED_BODY()

public:
    UGunSkill();

    void AttachGun(UGunWeapon* InGun) { Gun = InGun; }

    UPROPERTY(BlueprintAssignable, Category = "GunSkill")
    FOnGunSkillStateChanged OnGunSkillStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "GunSkill")
    FOnGunSkillCharged OnGunSkillCharged;

    UFUNCTION(BlueprintPure, Category = "GunSkill") int32 GetCurrentRicochetCount() const { return CurrentRicochetCount; }
    UFUNCTION(BlueprintPure, Category = "GunSkill") int32 GetMaxRicochetCount()     const { return MaxRicochetCount; }
    UFUNCTION(BlueprintPure, Category = "GunSkill") float GetChargeElapsed()        const { return ChargeElapsed; }
    UFUNCTION(BlueprintPure, Category = "GunSkill") float GetChargeProgress()       const;

protected:
    virtual void EndPlay(EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    virtual bool OnStartHold() override;
    virtual void OnEndHold()   override;
    virtual void OnCancel()    override;

private:
    int32 CalculateRicochetCount(float ElapsedSeconds) const;
    void  TickCharge(float UnscaledDelta);

    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float CooldownDuration = 5.f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float ChargedDamage    = 500.f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float FireLockout      = 0.5f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") int32 MaxRicochetCount = 4;

    UPROPERTY(EditDefaultsOnly, Category = "GunSkill")
    TArray<float> RicochetTimeThresholds = { 0.5f, 1.0f, 1.5f, 2.0f };

    UPROPERTY(EditDefaultsOnly, Category = "GunSkill|Anim") TObjectPtr<UAnimMontage> ChargeMontage;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill|Anim") TObjectPtr<UAnimMontage> FireMontage;

    UPROPERTY()
    TWeakObjectPtr<UGunWeapon> Gun;

    float ChargeElapsed        = 0.f;
    int32 CurrentRicochetCount = 0;
    int32 LastBroadcastedCount = -1;
};