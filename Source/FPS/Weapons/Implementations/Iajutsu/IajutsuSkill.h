#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponSkillBase.h"
#include "Combat/Data/WeaponHitResult.h"
#include "IajutsuSkill.generated.h"

class UAnimMontage;
class USwordWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIajutsuStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIajutsuEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIajutsuCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIajutsuHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UIajutsuSkill : public UWeaponSkillBase
{
    GENERATED_BODY()

public:
    UIajutsuSkill();

    void AttachSword(USwordWeapon* InSword) { Sword = InSword; }

    UPROPERTY(BlueprintAssignable, Category = "Iajutsu") FOnIajutsuStarted   OnIajutsuStarted;
    UPROPERTY(BlueprintAssignable, Category = "Iajutsu") FOnIajutsuEnded     OnIajutsuEnded;
    UPROPERTY(BlueprintAssignable, Category = "Iajutsu") FOnIajutsuCancelled OnIajutsuCancelled;
    UPROPERTY(BlueprintAssignable, Category = "Iajutsu") FOnIajutsuHit       OnIajutsuHit;

    UFUNCTION(BlueprintPure, Category = "Iajutsu") bool IsHolding() const { return bIsActive && !bIsDashing && !bIsStunned; }
    UFUNCTION(BlueprintPure, Category = "Iajutsu") bool IsDashing() const { return bIsDashing; }
    UFUNCTION(BlueprintPure, Category = "Iajutsu") bool IsStunned() const { return bIsStunned; }

    /** Iajutsu is "active" for skill purposes during hold, dash, AND stun (movement is locked). */
    virtual bool IsSkillActive_Implementation()  const override { return bIsActive || bIsDashing || bIsStunned; }
    virtual bool BlocksMovement_Implementation() const override { return IsSkillActive_Implementation(); }

protected:
    virtual void EndPlay(EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    virtual bool OnStartHold() override;
    virtual void OnEndHold()   override;
    virtual void OnCancel()    override;

private:
    void PerformDash();

    FVector CalculateDestination() const;
    void    HitActorsAlongPath(const FVector& Start, const FVector& End);

    void RequestTimeDilation();
    void ReleaseTimeDilation();

    void DisablePawnCollision();
    void RestorePawnCollision();

    void TickHold(float UnscaledDelta);
    void TickDash(float UnscaledDelta);
    void TickStun(float UnscaledDelta);

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float Damage             = 200.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float Distance           = 1500.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float CooldownDuration   = 6.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float DashDuration       = 0.1f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float StunDuration       = 0.3f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float HoldMaxDuration    = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu|Time") float SlowWorldDilation = 0.15f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu|Time") int32 DilationPriority  = 10;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu|Time") float DilationBlendIn   = 0.1f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu|Time") float DilationBlendOut  = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu|Anim") TObjectPtr<UAnimMontage> HoldMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu|Anim") TObjectPtr<UAnimMontage> DashMontage;

    UPROPERTY()
    TWeakObjectPtr<USwordWeapon> Sword;

    float HoldElapsed = 0.f;

    bool    bIsDashing      = false;
    float   DashElapsed     = 0.f;
    FVector DashStart       = FVector::ZeroVector;
    FVector DashDestination = FVector::ZeroVector;

    bool  bIsStunned  = false;
    float StunElapsed = 0.f;

    int32 TimeDilationHandle = 0;
};