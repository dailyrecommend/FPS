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
    if (bParryWindowOpen || bHitStopActive) return false;

    bParryWindowOpen   = true;
    ParryWindowElapsed = 0.f;

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
    StopMontage(SkillMontage);
    StartCooldown(CooldownDuration);
}

void USwordSkill::EndPlay(EEndPlayReason::Type Reason)
{
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

    ParryTarget = Attacker;

    // 윈도우 닫고 bIsActive 해제 (Cancel → OnCancel → StopMontage(ChargeMontage))
    Cancel();
    StartHitStop();

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
    // AttackMontage는 ChargeMontage와 별개 — Cancel로 중단되지 않음
    PlayMontageSection(SkillMontage, TEXT("Attack"));

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
    TickHitStop(UnscaledDelta);
    TickCooldown(UnscaledDelta);
}
