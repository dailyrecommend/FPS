#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FocusComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;
class UGunComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusStateChanged, bool, bIsFocusing);

/**
 * Manages the Focus (aim-down-sights / bullet-time) mechanic:
 * - Slows world time via TimeScaleComponent
 * - Interpolates camera FOV
 * - Fires a charged shot on release via GunComponent
 * - Tracks cooldown and max duration
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UFocusComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFocusComponent();

    void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera, UGunComponent* InGun);

    void StartFocus();
    void EndFocus();

    UPROPERTY(BlueprintAssignable)
    FOnFocusStateChanged OnFocusStateChanged;

    UFUNCTION(BlueprintPure) bool  IsFocusing()               const { return bIsFocusing; }
    UFUNCTION(BlueprintPure) bool  CanFocus()                 const;
    UFUNCTION(BlueprintPure) float GetFocusSensitivity()      const { return FocusSensitivity; }
    UFUNCTION(BlueprintPure) float GetCooldownRemaining()     const { return CooldownRemaining; }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void ApplyTimeScale();
    void TickFOV(float UnscaledDelta);
    void TickDuration(float UnscaledDelta);
    void TickCooldown(float UnscaledDelta);
    void PlayMontage(UAnimMontage* Montage);

    //~ Focus config
    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusWorldDilation  = 0.15f;

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusFOVOffset      = 30.f;       // Subtracted from default FOV

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusFOVInterpSpeed = 8.f;

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusSensitivity    = 0.4f;

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusMaxDuration    = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusCooldown       = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusChargedDamage  = 500.f;

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusStartDelay     = 0.05f;      // Delay before time scale kicks in

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    float FocusFireLockout    = 0.5f;       // Gun fire lockout after releasing focus

    //~ Animation
    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    UAnimMontage* FocusMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Focus")
    UAnimMontage* FocusFireMontage;

    UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
    UPROPERTY() UCameraComponent* Camera         = nullptr;
    UPROPERTY() UGunComponent*    Gun            = nullptr;

    bool  bIsFocusing     = false;
    float FocusElapsed    = 0.f;
    float CooldownRemaining = 0.f;
    float CurrentFOV      = 90.f;
    float TargetFOV       = 90.f;

    FTimerHandle FocusStartTimer;
};