#include "FocusComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GunComponent.h"
#include "TimeScaleComponent.h"

UFocusComponent::UFocusComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UFocusComponent::Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera, UGunComponent* InGun)
{
    OwnerCharacter = InOwner;
    Camera         = InCamera;
    Gun            = InGun;

    if (OwnerCharacter)
    {
        CurrentFOV = OwnerCharacter->GetDefaultFOV();
        TargetFOV  = CurrentFOV;
    }
}

bool UFocusComponent::CanFocus() const
{
    return !bIsFocusing && CooldownRemaining <= 0.f;
}

void UFocusComponent::StartFocus()
{
    if (!CanFocus() || !OwnerCharacter) return;

    bIsFocusing  = true;
    FocusElapsed = 0.f;
    TargetFOV    = OwnerCharacter->GetDefaultFOV() - FocusFOVOffset;

    PlayMontage(FocusMontage);
    OnFocusStateChanged.Broadcast(true);

    // Time scale kicks in after a brief delay so the animation can settle first
    OwnerCharacter->GetWorldTimerManager().SetTimer(FocusStartTimer, [this]()
    {
        if (!OwnerCharacter || !bIsFocusing) return;
        ApplyTimeScale();
    }, FocusStartDelay, false);
}

void UFocusComponent::EndFocus()
{
    OwnerCharacter->GetWorldTimerManager().ClearTimer(FocusStartTimer);

    if (!bIsFocusing || !OwnerCharacter) return;

    bIsFocusing       = false;
    CooldownRemaining = FocusCooldown;
    TargetFOV         = OwnerCharacter->GetDefaultFOV();

    OwnerCharacter->GetTimeScaleComponent()->ClearTimeScale(0.f);
    OnFocusStateChanged.Broadcast(false);

    // Stop focus montage and fire charged shot
    if (UAnimInstance* Anim = OwnerCharacter->GetArmsMesh()->GetAnimInstance())
    {
        Anim->Montage_Stop(0.1f, FocusMontage);
        PlayMontage(FocusFireMontage);
    }

    if (Gun)
    {
        Gun->PerformChargedShot(FocusChargedDamage, FocusFireLockout);
    }
}

void UFocusComponent::ApplyTimeScale()
{
    if (!OwnerCharacter) return;

    FTimeScaleParams Params;
    Params.Mode          = ETimeScaleMode::Full;
    Params.WorldDilation = FocusWorldDilation;
    Params.Duration      = 0.f;
    Params.BlendIn       = 0.1f;
    Params.BlendOut      = 0.2f;

    OwnerCharacter->GetTimeScaleComponent()->ApplyTimeScale(Params);
}

void UFocusComponent::PlayMontage(UAnimMontage* Montage)
{
    if (!Montage || !OwnerCharacter) return;

    USkeletalMeshComponent* Arms = OwnerCharacter->GetArmsMesh();
    if (!Arms) return;

    UAnimInstance* Anim = Arms->GetAnimInstance();
    if (Anim) Anim->Montage_Play(Montage, 1.f);
}

void UFocusComponent::TickFOV(float UnscaledDelta)
{
    if (!Camera) return;
    CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, UnscaledDelta, FocusFOVInterpSpeed);
    Camera->SetFieldOfView(CurrentFOV);
}

void UFocusComponent::TickDuration(float UnscaledDelta)
{
    if (!bIsFocusing) return;
    FocusElapsed += UnscaledDelta;
    if (FocusElapsed >= FocusMaxDuration)
        EndFocus();
}

void UFocusComponent::TickCooldown(float UnscaledDelta)
{
    if (CooldownRemaining > 0.f)
        CooldownRemaining = FMath::Max(0.f, CooldownRemaining - UnscaledDelta);
}

void UFocusComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const float UnscaledDelta = FApp::GetDeltaTime();
    TickFOV(UnscaledDelta);
    TickDuration(UnscaledDelta);
    TickCooldown(UnscaledDelta);
}