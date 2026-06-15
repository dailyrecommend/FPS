#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponSkillBase.h"
#include "SwordSkill.generated.h"

class UAnimMontage;
class USwordWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParrySuccess);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USwordSkill : public UWeaponSkillBase
{
    GENERATED_BODY()

public:
    USwordSkill();

    void AttachSword(USwordWeapon* InSword) { Sword = InSword; }

    /**
     * 플레이어 피격 처리 측에서 호출.
     * 패링 윈도우가 열려 있고 정면 공격이면 피격을 소비하고 true 반환.
     */
    UFUNCTION(BlueprintCallable, Category = "SwordSkill")
    bool TryParry(AActor* Attacker);

    UFUNCTION(BlueprintPure, Category = "SwordSkill")
    bool IsParryWindowOpen() const { return bParryWindowOpen; }

    UPROPERTY(BlueprintAssignable, Category = "SwordSkill")
    FOnParrySuccess OnParrySuccess;

    virtual bool IsSkillActive_Implementation()  const override { return bParryWindowOpen || bHitStopActive; }
    virtual bool BlocksMovement_Implementation() const override { return bParryWindowOpen || bHitStopActive; }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(EEndPlayReason::Type Reason) override;

    virtual bool OnStartHold() override;
    virtual void OnEndHold()   override;
    virtual void OnCancel()    override;

    // 클릭 기반 스킬 — 버튼 릴리즈로 스킬이 종료되지 않음
    // 종료는 윈도우 만료(Cancel) 또는 패링 성공 후 히트 스탑 종료 시에만 발생
    virtual void EndHold_Implementation() override {};

private:
    bool IsAttackerInFront(AActor* Attacker) const;
    void CloseParryWindow();
    void StartHitStop();
    void EndHitStop();
    void ExecuteCounterAttack();

    void TickParryWindow(float UnscaledDelta);
    void TickHitStop(float UnscaledDelta);

    bool  bParryWindowOpen   = false;
    float ParryWindowElapsed = 0.f;

    bool  bHitStopActive = false;
    float HitStopElapsed = 0.f;

    int32 HitStopDilationHandle = 0;

    UPROPERTY()
    TWeakObjectPtr<AActor> ParryTarget;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill")
    float CooldownDuration = 8.f;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill")
    float ParryWindowDuration = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill")
    float HitStopDuration = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Time")
    float HitStopTimeDilation = 0.01f;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Time")
    int32 HitStopDilationPriority = 20;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill")
    float CounterDamage = 300.f;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Camera")
    float CameraKickbackAmount = 15.f;

    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Camera")
    float CameraKickbackDuration = 0.3f;

    /** Charge 섹션: 패링 시전 / Attack 섹션: 반격 */
    UPROPERTY(EditDefaultsOnly, Category = "SwordSkill|Anim")
    TObjectPtr<UAnimMontage> SkillMontage;

    UPROPERTY()
    TWeakObjectPtr<USwordWeapon> Sword;
};
