#include "Weapons/Implementations/GunSkill/GunSkill.h"
#include "Weapons/Implementations/Gun/GunWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Misc/App.h"

UGunSkill::UGunSkill()
{
}


void UGunSkill::OnEndHold()
{
    StartCooldown(CooldownDuration);

    ResetGunRotation();

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
    ResetGunRotation();
    StopMontage(GunSkillMontage);

    ChargeElapsed        = 0.f;
    CurrentRicochetCount = 0;
    LastBroadcastedCount = -1;

    OnGunSkillStateChanged.Broadcast(false);
}

void UGunSkill::EndPlay(EEndPlayReason::Type Reason)
{
    ResetGunRotation();
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

USkeletalMeshComponent* UGunSkill::FindWeaponMesh() const
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return nullptr;

    return Cast<USkeletalMeshComponent>(
        Owner->GetDefaultSubobjectByName(WeaponMeshComponentName));
}

bool UGunSkill::OnStartHold()
{
    ChargeElapsed        = 0.f;
    CurrentRicochetCount = 0;
    LastBroadcastedCount = -1;
    AccumulatedZRotation = 0.f;

    USkeletalMeshComponent* Mesh = FindWeaponMesh();
    if (Mesh)
        Mesh->SetRelativeRotation(GunInitialRotation);

    PlayMontageSection(GunSkillMontage, TEXT("Charge"));
    OnGunSkillStateChanged.Broadcast(true);
    return true;
}

void UGunSkill::TickGunRotation(float DeltaTime)
{
    if (!bIsActive) return;

    USkeletalMeshComponent* Mesh = FindWeaponMesh();
    if (!Mesh) return;

    const float RotAmount = GunRotationSpeed * DeltaTime;
    AccumulatedZRotation += RotAmount;

    Mesh->AddLocalRotation(FQuat(FVector::UpVector, FMath::DegreesToRadians(RotAmount)));
}

void UGunSkill::ResetGunRotation()
{
    USkeletalMeshComponent* Mesh = FindWeaponMesh();
    if (!Mesh) return;

    Mesh->SetRelativeRotation(GunInitialRotation);
    AccumulatedZRotation = 0.f;
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
    TickGunRotation(DeltaTime);
}