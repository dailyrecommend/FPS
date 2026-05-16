#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponSkillBase.h"
#include "GunSkill.generated.h"

class UAnimMontage;
class UGunWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunSkillStateChanged, bool, bIsActive);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UGunSkill : public UWeaponSkillBase
{
    GENERATED_BODY()

public:
    UGunSkill();

    void AttachGun(UGunWeapon* InGun) { Gun = InGun; }

    UPROPERTY(BlueprintAssignable, Category = "GunSkill")
    FOnGunSkillStateChanged OnGunSkillStateChanged;

    UFUNCTION(BlueprintPure, Category = "GunSkill") float GetSensitivity() const { return Sensitivity; }

protected:
    virtual void EndPlay(EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    virtual bool OnStartHold() override;
    virtual void OnEndHold()   override;
    virtual void OnCancel()    override;

private:
    void RequestTimeDilationDelayed();
    void RequestTimeDilation();
    void ReleaseTimeDilation();
    void TickDuration(float UnscaledDelta);

    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float WorldDilation    = 0.15f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") int32 DilationPriority = 5;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float DilationBlendIn  = 0.1f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float DilationBlendOut = 0.2f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float StartDelay       = 0.05f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float MaxDuration      = 3.f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float CooldownDuration = 5.f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float ChargedDamage    = 500.f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float FireLockout      = 0.5f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill") float Sensitivity      = 0.4f;

    UPROPERTY(EditDefaultsOnly, Category = "GunSkill|Camera") float FOVOffset      = -30.f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill|Camera") float FOVInterpSpeed = 8.f;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill|Camera") int32 FOVPriority    = 5;

    UPROPERTY(EditDefaultsOnly, Category = "GunSkill|Anim") TObjectPtr<UAnimMontage> ChargeMontage;
    UPROPERTY(EditDefaultsOnly, Category = "GunSkill|Anim") TObjectPtr<UAnimMontage> FireMontage;

    UPROPERTY()
    TWeakObjectPtr<UGunWeapon> Gun;

    float Elapsed            = 0.f;
    int32 TimeDilationHandle = 0;
    int32 FOVHandle          = 0;

    FTimerHandle StartDelayTimer;
};