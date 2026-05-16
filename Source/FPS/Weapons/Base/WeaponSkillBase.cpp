#include "Weapons/Base/WeaponSkillBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"

UWeaponSkillBase::UWeaponSkillBase()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWeaponSkillBase::InjectDependencies(ACharacter* InOwner, UCameraComponent* InCamera)
{
    OwnerCharacter = InOwner;
    Camera         = InCamera;
}

bool UWeaponSkillBase::CanActivate() const
{
    return !bIsActive && CooldownRemaining <= 0.f && OwnerCharacter.IsValid();
}

void UWeaponSkillBase::StartHold_Implementation()
{
    if (!CanActivate()) return;
    if (!OnStartHold()) return;
    SetActiveInternal(true);
}

void UWeaponSkillBase::EndHold_Implementation()
{
    if (!bIsActive) return;
    OnEndHold();
    SetActiveInternal(false);
}

void UWeaponSkillBase::Cancel()
{
    if (!bIsActive) return;
    OnCancel();
    SetActiveInternal(false);
}

void UWeaponSkillBase::TickCooldown(float UnscaledDelta)
{
    if (CooldownRemaining > 0.f)
        CooldownRemaining = FMath::Max(0.f, CooldownRemaining - UnscaledDelta);
}

void UWeaponSkillBase::StartCooldown(float Duration)
{
    CooldownRemaining = FMath::Max(0.f, Duration);
}

void UWeaponSkillBase::PlayMontageSection(UAnimMontage* Montage, FName SectionName, float PlayRate)
{
    if (!Montage) return;
    UObject* Player = AnimationPlayer.GetObject();
    if (!Player) return;
    IAnimationPlayer::Execute_PlayMontageSection(Player, Montage, SectionName, PlayRate);
}

void UWeaponSkillBase::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage) return;
    UObject* Player = AnimationPlayer.GetObject();
    if (!Player) return;
    IAnimationPlayer::Execute_PlayMontage(Player, Montage, PlayRate);
}

void UWeaponSkillBase::StopMontage(UAnimMontage* Montage, float BlendOutTime)
{
    if (!Montage) return;
    UObject* Player = AnimationPlayer.GetObject();
    if (!Player) return;
    IAnimationPlayer::Execute_StopMontage(Player, Montage, BlendOutTime);
}

int32 UWeaponSkillBase::PushFOVOffset(float Offset, float InterpSpeed, int32 Priority)
{
    UObject* Effects = CameraEffects.GetObject();
    if (!Effects) return 0;
    return ICameraEffects::Execute_PushFOVOffset(Effects, Offset, InterpSpeed, Priority);
}

void UWeaponSkillBase::PopFOVOffset(int32 Handle)
{
    if (Handle == 0) return;
    UObject* Effects = CameraEffects.GetObject();
    if (!Effects) return;
    ICameraEffects::Execute_PopFOVOffset(Effects, Handle);
}

ACharacter* UWeaponSkillBase::GetOwnerSafe() const
{
    return OwnerCharacter.Get();
}

UCameraComponent* UWeaponSkillBase::GetCameraSafe() const
{
    return Camera.Get();
}

void UWeaponSkillBase::SetActiveInternal(bool bNewActive)
{
    if (bIsActive == bNewActive) return;
    bIsActive = bNewActive;
    OnSkillStateChanged.Broadcast(bIsActive);
}