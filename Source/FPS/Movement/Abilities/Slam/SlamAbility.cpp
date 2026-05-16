#include "Movement/Abilities/Slam/SlamAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USlamAbility::USlamAbility()
{
    AbilityId = TEXT("Slam");
    Cooldown  = 0.f;
}

bool USlamAbility::CheckPreconditions(const FAbilityContext& /*Context*/) const
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return false;
    return !MoveComp->IsMovingOnGround() && !bIsActive && !bIsStunned;
}

EActivationResult USlamAbility::OnTryActivate(const FAbilityContext& /*Context*/)
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return EActivationResult::Failed_NoOwner;

    MoveComp->Velocity = FVector(0.f, 0.f, -SlamDownForce);

    PlayMontage(SlamMontage);
    return EActivationResult::Success;
}

void USlamAbility::OnDeactivate()
{
    bIsStunned = false;
    StunTimer  = 0.f;
    RestoreMovementMode();
}

bool USlamAbility::CanBeInterruptedBy_Implementation(FName OtherAbilityId) const
{
    return OtherAbilityId == DashAbilityIdAsInterruptor;
}

void USlamAbility::TickComponent(float DeltaTime, ELevelTick TickType,
                                 FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return;

    if (bIsActive && !bIsStunned && MoveComp->IsMovingOnGround())
    {
        OnSlamLanded();
        return;
    }

    TickStun(DeltaTime);
}

void USlamAbility::OnSlamLanded()
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return;

    bIsStunned = true;
    StunTimer  = SlamLandingStunDuration;
    MoveComp->DisableMovement();
}

void USlamAbility::TickStun(float DeltaTime)
{
    if (!bIsStunned) return;

    StunTimer -= DeltaTime;
    if (StunTimer <= 0.f)
    {
        bIsStunned = false;
        RestoreMovementMode();
        Deactivate_Implementation();
    }
}

void USlamAbility::RestoreMovementMode()
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return;
    MoveComp->SetMovementMode(MOVE_Walking);
}