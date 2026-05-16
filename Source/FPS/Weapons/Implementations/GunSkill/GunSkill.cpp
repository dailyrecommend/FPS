#include "Weapons/Implementations/GunSkill/GunSkill.h"
#include "Weapons/Implementations/Gun/GunWeapon.h"
#include "Misc/App.h"

UGunSkill::UGunSkill()
{
}

bool UGunSkill::OnStartHold()
{
    ChargeElapsed        = 0.f;
    CurrentRicochetCount = 0;
    LastBroadcastedCount = -1;

    PlayMontage(ChargeMontage);
    OnGunSkillStateChanged.Broadcast(true);
    return true;
}

void UGunSkill::OnEndHold()
{
    StartCooldown(CooldownDuration);

    StopMontage(ChargeMontage);
    PlayMontage(FireMontage);

    if (UGunWeapon* G = Gun.Get())
        G->FireRicochetShot(ChargedDamage, CurrentRicochetCount, FireLockout);

    ChargeElapsed        = 0.f;
    CurrentRicochetCount = 0;
    LastBroadcastedCount = -1;

    OnGunSkillStateChanged.Broadcast(false);
}

void UGunSkill::OnCancel()
{
    StopMontage(ChargeMontage);

    ChargeElapsed        = 0.f;
    CurrentRicochetCount = 0;
    LastBroadcastedCount = -1;

    OnGunSkillStateChanged.Broadcast(false);
}

void UGunSkill::EndPlay(EEndPlayReason::Type Reason)
{
    Super::EndPlay(Reason);
}

float UGunSkill::GetChargeProgress() const
{
    if (RicochetTimeThresholds.Num() == 0) return 0.f;
    return FMath::Clamp(ChargeElapsed / RicochetTimeThresholds.Last(), 0.f, 1.f);
}

int32 UGunSkill::CalculateRicochetCount(float ElapsedSeconds) const
{
    int32 Count = 0;
    for (float Threshold : RicochetTimeThresholds)
    {
        if (ElapsedSeconds >= Threshold) Count++;
        else break;
    }
    return FMath::Min(Count, MaxRicochetCount);
}

void UGunSkill::TickCharge(float UnscaledDelta)
{
    if (!bIsActive) return;

    ChargeElapsed += UnscaledDelta;

    const int32 NewCount = CalculateRicochetCount(ChargeElapsed);
    if (NewCount != LastBroadcastedCount)
    {
        CurrentRicochetCount = NewCount;
        LastBroadcastedCount = NewCount;
        OnGunSkillCharged.Broadcast(CurrentRicochetCount);
    }
}

void UGunSkill::TickComponent(float DeltaTime, ELevelTick TickType,
                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const float UnscaledDelta = FApp::GetDeltaTime();
    TickCharge(UnscaledDelta);
    TickCooldown(UnscaledDelta);
}