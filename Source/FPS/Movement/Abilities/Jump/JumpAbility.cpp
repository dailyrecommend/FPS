#include "Movement/Abilities/Jump/JumpAbility.h"
#include "Movement/Registry/AbilityRegistry.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UJumpAbility::UJumpAbility()
{
    AbilityId = TEXT("Jump");
    Cooldown  = 0.f;
}

UAbilityRegistry* UJumpAbility::GetRegistry() const
{
    AActor* Owner = GetOwner();
    return Owner ? Owner->FindComponentByClass<UAbilityRegistry>() : nullptr;
}

EActivationResult UJumpAbility::OnTryActivate(const FAbilityContext& Context)
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) return EActivationResult::Failed_NoOwner;

    if (TrySlideExitJump())             { Deactivate_Implementation(); return EActivationResult::Success; }
    if (TryGroundJump())                { Deactivate_Implementation(); return EActivationResult::Success; }
    if (TryDelegateWallJump(Context))   { Deactivate_Implementation(); return EActivationResult::Success; }
    if (TryCoyoteJump())                { Deactivate_Implementation(); return EActivationResult::Success; }

    BufferJump();
    Deactivate_Implementation();
    return EActivationResult::Failed_NotReady;
}

bool UJumpAbility::TrySlideExitJump()
{
    UAbilityRegistry* Registry = GetRegistry();
    ACharacter* Owner = GetOwnerSafe();
    if (!Registry || !Owner) return false;

    if (!Registry->IsAbilityActive(SlideAbilityId)) return false;

    Registry->Cancel(SlideAbilityId);
    Owner->Jump();
    return true;
}

bool UJumpAbility::TryGroundJump()
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) return false;

    if (!MoveComp->IsMovingOnGround()) return false;

    Owner->Jump();
    return true;
}

bool UJumpAbility::TryDelegateWallJump(const FAbilityContext& Context)
{
    UAbilityRegistry* Registry = GetRegistry();
    if (!Registry) return false;

    return Registry->TryActivate(WallJumpAbilityId, Context) == EActivationResult::Success;
}

bool UJumpAbility::TryCoyoteJump()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return false;

    if (!CanCoyoteJump()) return false;

    bCoyoteJumpUsed   = true;
    CoyoteTimeCounter = 0.f;
    Owner->Jump();
    return true;
}

void UJumpAbility::BufferJump()
{
    bJumpBuffered     = true;
    JumpBufferCounter = JumpBufferDuration;
}

bool UJumpAbility::CanCoyoteJump() const
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return false;
    return !MoveComp->IsMovingOnGround() && CoyoteTimeCounter > 0.f && !bCoyoteJumpUsed;
}

void UJumpAbility::TickComponent(float DeltaTime, ELevelTick TickType,
                                 FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickCoyoteTime(DeltaTime);
    TickJumpBuffer(DeltaTime);
}

void UJumpAbility::TickCoyoteTime(float DeltaTime)
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return;

    const bool bIsGrounded = MoveComp->IsMovingOnGround();

    if (bWasGrounded && !bIsGrounded)
    {
        CoyoteTimeCounter = CoyoteTimeDuration;
        bCoyoteJumpUsed   = false;
    }

    if (bIsGrounded)
    {
        CoyoteTimeCounter = 0.f;
        bCoyoteJumpUsed   = false;
    }
    else if (CoyoteTimeCounter > 0.f)
    {
        CoyoteTimeCounter -= DeltaTime;
    }

    bWasGrounded = bIsGrounded;
}

void UJumpAbility::TickJumpBuffer(float DeltaTime)
{
    if (!bJumpBuffered) return;

    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) { bJumpBuffered = false; return; }

    JumpBufferCounter -= DeltaTime;

    if (MoveComp->IsMovingOnGround())
    {
        Owner->Jump();
        bJumpBuffered     = false;
        JumpBufferCounter = 0.f;
        return;
    }

    if (JumpBufferCounter <= 0.f)
    {
        bJumpBuffered     = false;
        JumpBufferCounter = 0.f;
    }
}