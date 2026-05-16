#include "Weapons/Implementations/GunSkill/GunSkill.h"

#include "GameFramework/Character.h"
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
    CurrentGunRoll       = 0.f;  // 회전 초기화

    PlayMontageSection(GunSkillMontage, TEXT("Charge"));
    OnGunSkillStateChanged.Broadcast(true);
    return true;
}


void UGunSkill::OnEndHold()
{
    StartCooldown(CooldownDuration);

    ResetGunRotation();  // 회전 복구

    PlayMontageSection(GunSkillMontage, TEXT("Fire"));

    if (UGunWeapon* G = Gun.Get())
        G->FireRicochetShot(ChargedDamage, CurrentRicochetCount, FireLockout);

    ChargeElapsed        = 0.f;
    CurrentRicochetCount = 0;
    LastBroadcastedCount = -1;

    OnGunSkillStateChanged.Broadcast(false);
}

void UGunSkill::OnCancel()
{
    ResetGunRotation();  // 회전 복구

    StopMontage(GunSkillMontage);

    ChargeElapsed        = 0.f;
    CurrentRicochetCount = 0;
    LastBroadcastedCount = -1;

    OnGunSkillStateChanged.Broadcast(false);
}

void UGunSkill::TickGunRotation(float DeltaTime)
{
    if (!bIsActive) return;

    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return;

    USkeletalMeshComponent* ArmsMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!ArmsMesh) return;

    CurrentGunRoll += GunRotationSpeed * DeltaTime;

    const FRotator CurrentRot = ArmsMesh->GetRelativeRotation();
    ArmsMesh->SetRelativeRotation(FRotator(
        CurrentRot.Pitch,
        CurrentRot.Yaw,
        CurrentGunRoll
    ));
}

void UGunSkill::ResetGunRotation()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return;

    USkeletalMeshComponent* ArmsMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!ArmsMesh) return;

    const FRotator CurrentRot = ArmsMesh->GetRelativeRotation();
    ArmsMesh->SetRelativeRotation(FRotator(
        CurrentRot.Pitch,
        CurrentRot.Yaw,
        0.f
    ));

    CurrentGunRoll = 0.f;
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
    TickGunRotation(DeltaTime);  // 회전은 DeltaTime 사용
}