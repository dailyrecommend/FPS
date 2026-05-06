#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponSkillBase.h"
#include "FocusSkill.generated.h"

class UAnimMontage;
class UGunWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusStateChanged, bool, bIsFocusing);

/**
 * Focus skill. On hold: requests slow-mo + FOV pinch. On release: fires a charged shot
 * through the injected UGunWeapon. All visual concerns go through the presentation
 * interfaces inherited from the base.
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UFocusSkill : public UWeaponSkillBase
{
    GENERATED_BODY()

public:
    UFocusSkill();

    void AttachGun(UGunWeapon* InGun) { Gun = InGun; }

    UPROPERTY(BlueprintAssignable, Category = "Focus")
    FOnFocusStateChanged OnFocusStateChanged;

    UFUNCTION(BlueprintPure, Category = "Focus") float GetSensitivity() const { return Sensitivity; }

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

    UPROPERTY(EditDefaultsOnly, Category = "Focus") float WorldDilation     = 0.15f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus") int32 DilationPriority  = 5;
    UPROPERTY(EditDefaultsOnly, Category = "Focus") float DilationBlendIn   = 0.1f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus") float DilationBlendOut  = 0.2f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus") float StartDelay        = 0.05f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus") float MaxDuration       = 3.f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus") float CooldownDuration  = 5.f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus") float ChargedDamage     = 500.f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus") float FireLockout       = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Focus|Camera") float FOVOffset       = -30.f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus|Camera") float FOVInterpSpeed  = 8.f;
    UPROPERTY(EditDefaultsOnly, Category = "Focus|Camera") int32 FOVPriority     = 5;

    UPROPERTY(EditDefaultsOnly, Category = "Focus|Input") float Sensitivity = 0.4f;

    UPROPERTY(EditDefaultsOnly, Category = "Focus|Anim") TObjectPtr<UAnimMontage> FocusMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Focus|Anim") TObjectPtr<UAnimMontage> FocusFireMontage;

    UPROPERTY()
    TWeakObjectPtr<UGunWeapon> Gun;

    float Elapsed            = 0.f;
    int32 TimeDilationHandle = 0;
    int32 FOVHandle          = 0;

    FTimerHandle StartDelayTimer;
};