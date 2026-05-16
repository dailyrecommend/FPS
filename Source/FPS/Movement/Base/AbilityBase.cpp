#include "Movement/Base/AbilityBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UAbilityBase::UAbilityBase()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UAbilityBase::InjectDependencies(ACharacter* InOwner)
{
    OwnerCharacter = InOwner;
}

EActivationResult UAbilityBase::TryActivate_Implementation(const FAbilityContext& Context)
{
    if (!OwnerCharacter.IsValid())
        return EActivationResult::Failed_NoOwner;

    if (!IsCooldownReady())
        return EActivationResult::Failed_OnCooldown;

    if (!CheckPreconditions(Context))
        return EActivationResult::Failed_NotReady;

    const EActivationResult Result = OnTryActivate(Context);
    if (Result == EActivationResult::Success)
    {
        StartCooldown();
        SetActiveInternal(true);
    }
    return Result;
}

void UAbilityBase::Deactivate_Implementation()
{
    if (!bIsActive) return;
    OnDeactivate();
    SetActiveInternal(false);
}

bool UAbilityBase::RequestCancel_Implementation()
{
    if (!bIsActive) return false;
    Deactivate_Implementation();
    return true;
}

bool UAbilityBase::IsCooldownReady() const
{
    if (Cooldown <= 0.f) return true;
    if (!OwnerCharacter.IsValid() || !OwnerCharacter->GetWorld()) return false;
    return (OwnerCharacter->GetWorld()->GetTimeSeconds() - LastActivationTime) >= Cooldown;
}

void UAbilityBase::StartCooldown()
{
    if (OwnerCharacter.IsValid() && OwnerCharacter->GetWorld())
        LastActivationTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
}

void UAbilityBase::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage) return;
    UObject* Player = AnimationPlayer.GetObject();
    if (!Player) return;
    IAnimationPlayer::Execute_PlayMontage(Player, Montage, PlayRate);
}

void UAbilityBase::StopMontage(UAnimMontage* Montage, float BlendOutTime)
{
    if (!Montage) return;
    UObject* Player = AnimationPlayer.GetObject();
    if (!Player) return;
    IAnimationPlayer::Execute_StopMontage(Player, Montage, BlendOutTime);
}

UCharacterMovementComponent* UAbilityBase::GetMoveComp() const
{
    ACharacter* Owner = OwnerCharacter.Get();
    return Owner ? Owner->GetCharacterMovement() : nullptr;
}

ACharacter* UAbilityBase::GetOwnerSafe() const
{
    return OwnerCharacter.Get();
}

void UAbilityBase::SetActiveInternal(bool bNewActive)
{
    if (bIsActive == bNewActive) return;
    bIsActive = bNewActive;
    OnAbilityStateChanged.Broadcast(bIsActive);
}