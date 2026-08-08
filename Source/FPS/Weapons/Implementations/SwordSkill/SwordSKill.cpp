#include "Weapons/Implementations/SwordSkill/SwordSkill.h"
#include "Weapons/Implementations/Sword/SwordWeapon.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Combat/Interfaces/Damageable.h"
#include "Core/Subsystems/TimeDilationSubsystem.h"
#include "Core/Data/TimeDilationRequest.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Misc/App.h"

USwordSkill::USwordSkill()
{
}

// ──────────────────────────────────────────────
// WeaponSkillBase overrides
// ──────────────────────────────────────────────

bool USwordSkill::OnStartHold()
{
    if (bParryWindowOpen || bHitStopPending || bHitStopActive) return false;

    bParryWindowOpen   = true;
    ParryWindowElapsed = 0.f;
    bParryConsumed     = false;

    PlayMontageSection(SkillMontage, TEXT("Charge"));
    return true;
}

void USwordSkill::OnEndHold()
{
    // 버튼을 놓아도 패링 윈도우는 만료 또는 패링 성공까지 유지
}

void USwordSkill::OnCancel()
{
    CloseParryWindow();

    // 패링 성공으로 끝난 경우엔 몽타주를 끊지 않는다.
    // 끊으면 기본 포즈로 블렌드된 뒤 Attack이 나와서 동작이 끊겨 보인다.
    if (!bParryConsumed)
        StopMontage(SkillMontage);

    StartCooldown(CooldownDuration);
}

void USwordSkill::EndPlay(EEndPlayReason::Type Reason)
{
    bHitStopPending     = false;
    HitStopDelayElapsed = 0.f;

    EndHitStop();
    Super::EndPlay(Reason);
}

// ──────────────────────────────────────────────
// 패링 판정
// ──────────────────────────────────────────────

bool USwordSkill::TryParry(AActor* Attacker)
{
    if (!bParryWindowOpen) return false;
    if (!IsAttackerInFront(Attacker)) return false;

    ParryTarget    = Attacker;
    bParryConsumed = true;

    // 윈도우를 닫고 bIsActive를 내린다. bParryConsumed 덕에 몽타주는 유지된다.
    Cancel();

    // Attack을 히트 스탑 뒤가 아니라 지금 재생한다. 시간 지연이 WorldOnly라
    // 플레이어만 정상 속도로 움직여, 멈춘 월드 위에서 반격 모션이 나온다.
    PlayMontageSection(SkillMontage, TEXT("Attack"));

    // 히트 스탑은 곧바로가 아니라 HitStopDelay 뒤에 건다 — 반격의 타격 순간에 맞추기 위해.
    if (HitStopDelay > 0.f)
    {
        bHitStopPending     = true;
        HitStopDelayElapsed = 0.f;
    }
    else
    {
        StartHitStop();
    }

    OnParrySuccess.Broadcast();
    return true;
}

bool USwordSkill::IsAttackerInFront(AActor* Attacker) const
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Attacker) return false;

    const FVector ToAttacker = (Attacker->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    return FVector::DotProduct(Owner->GetActorForwardVector(), ToAttacker) > 0.f;
}

// ──────────────────────────────────────────────
// 패링 윈도우 관리
// ──────────────────────────────────────────────

void USwordSkill::CloseParryWindow()
{
    bParryWindowOpen   = false;
    ParryWindowElapsed = 0.f;
}

// ──────────────────────────────────────────────
// 히트 스탑 (시간 정지 + 카메라 킥백)
// ──────────────────────────────────────────────

void USwordSkill::StartHitStop()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    bHitStopActive = true;
    HitStopElapsed = 0.f;

    if (UTimeDilationSubsystem* TimeSys = Owner->GetWorld()->GetSubsystem<UTimeDilationSubsystem>())
    {
        FTimeDilationRequest Request;
        Request.WorldDilation = HitStopTimeDilation;
        Request.BlendIn       = 0.f;
        Request.BlendOut      = 0.f;
        Request.Priority      = HitStopDilationPriority;
        Request.Requester     = Owner;

        HitStopDilationHandle = TimeSys->PushRequest(Request);
    }

    TriggerCameraPositionKickback(CameraKickbackAmount, CameraKickbackDuration);
}

void USwordSkill::EndHitStop()
{
    if (!bHitStopActive) return;

    bHitStopActive = false;
    HitStopElapsed = 0.f;
    bParryConsumed = false;

    ACharacter* Owner = GetOwnerSafe();
    if (Owner && Owner->GetWorld())
    {
        if (UTimeDilationSubsystem* TimeSys = Owner->GetWorld()->GetSubsystem<UTimeDilationSubsystem>())
            TimeSys->PopRequest(HitStopDilationHandle);
    }
    HitStopDilationHandle = 0;

    ExecuteCounterAttack();
    StartCooldown(CooldownDuration);
}

// ──────────────────────────────────────────────
// 반격
// ──────────────────────────────────────────────

void USwordSkill::ExecuteCounterAttack()
{
    // 모션은 TryParry에서 이미 재생됐다. 여기서는 피해만 적용한다.
    AActor* Target = ParryTarget.Get();
    if (!Target || !Target->Implements<UDamageable>()) return;

    ACharacter* Owner = GetOwnerSafe();

    FHitResultBuilder()
        .From(Owner ? Owner->GetController() : nullptr)
        .Target(Target)
        .At(Target->GetActorLocation(), FVector::ZeroVector)
        .WithDamage(CounterDamage)
        .OfDamageType(EWeaponDamageType::Sword)
        .OfHitType(EHitType::Parried)
        .Apply();

    ParryTarget = nullptr;
}

// ──────────────────────────────────────────────
// Tick
// ──────────────────────────────────────────────

void USwordSkill::TickParryWindow(float UnscaledDelta)
{
    if (!bParryWindowOpen) return;

    ParryWindowElapsed += UnscaledDelta;
    if (ParryWindowElapsed >= ParryWindowDuration)
        Cancel();
}

void USwordSkill::TickHitStopDelay(float UnscaledDelta)
{
    if (!bHitStopPending) return;

    HitStopDelayElapsed += UnscaledDelta;
    if (HitStopDelayElapsed >= HitStopDelay)
    {
        bHitStopPending     = false;
        HitStopDelayElapsed = 0.f;
        StartHitStop();
    }
}

void USwordSkill::TickHitStop(float UnscaledDelta)
{
    if (!bHitStopActive) return;

    HitStopElapsed += UnscaledDelta;
    if (HitStopElapsed >= HitStopDuration)
        EndHitStop();
}

void USwordSkill::TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const float UnscaledDelta = FApp::GetDeltaTime();
    TickParryWindow(UnscaledDelta);
    TickHitStopDelay(UnscaledDelta);
    TickHitStop(UnscaledDelta);
    TickCooldown(UnscaledDelta);
}
