#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponSkillBase.h"
#include "Combat/Data/WeaponHitResult.h"
#include "SwordSkill.generated.h"

class UAnimMontage;
class USwordWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwordSkillStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwordSkillEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwordSkillCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwordSkillHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USwordSkill : public UWeaponSkillBase
{
    GENERATED_BODY()

public:
    USwordSkill();

    void AttachSword(USwordWeapon* InSword) { Sword = InSword; }

    UPROPERTY(BlueprintAssignable, Category = "SwordSkill") FOnSwordSkillStarted   OnSwordSkillStarted;
    UPROPERTY(BlueprintAssignable, Category = "SwordSkill") FOnSwordSkillEnded     OnSwordSkillEnded;
    UPROPERTY(BlueprintAssignable, Category = "SwordSkill") FOnSwordSkillCancelled OnSwordSkillCancelled;
    UPROPERTY(BlueprintAssignable, Category = "SwordSkill") FOnSwordSkillHit       OnSwordSkillHit;

    UFUNCTION(BlueprintPure, Category = "SwordSkill") bool IsHolding() const { return bIsActive && !bIsDashing && !bIsStunned; }
    UFUNCTION(BlueprintPure, Category = "SwordSkill") bool IsDashing() const { return bIsDashing; }
    UFUNCTION(BlueprintPure, Category = "SwordSkill") bool IsStunned() const { return bIsStunned; }

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

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill") float Damage           = 200.f;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill") float Distance         = 1500.f;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill") float CooldownDuration = 6.f;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill") float DashDuration     = 0.1f;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill") float StunDuration     = 0.3f;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill") float HoldMaxDuration  = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Time") float SlowWorldDilation = 0.15f;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Time") int32 DilationPriority  = 10;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Time") float DilationBlendIn   = 0.1f;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Time") float DilationBlendOut  = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Anim") TObjectPtr<UAnimMontage> HoldMontage;
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Anim") TObjectPtr<UAnimMontage> DashMontage;

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