#include "JumpComponent.h"
#include "../PlayerCharacter.h"
#include "WallJumpComponent.h"
#include "GlissandoComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UJumpComponent::UJumpComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UJumpComponent::Initialize(APlayerCharacter* InOwner, UWallJumpComponent* InWallJump, UGlissandoComponent* InGlissando)
{
    OwnerCharacter = InOwner;
    WallJump       = InWallJump;
    Glissando      = InGlissando;
}

void UJumpComponent::TryJump()
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();

    // Glissando jump: exit slide and jump
    if (Glissando && Glissando->IsGlissando())
    {
        Glissando->EndGlissando();
        OwnerCharacter->Jump();
        return;
    }

    // Standard ground jump
    if (MoveComp->IsMovingOnGround())
    {
        OwnerCharacter->Jump();
        return;
    }

    // Wall jump
    if (WallJump && WallJump->IsOnWall() && WallJump->TryWallJump())
    {
        return;
    }

    // Coyote time jump
    if (CanCoyoteJump())
    {
        bCoyoteJumpUsed   = true;
        CoyoteTimeCounter = 0.f;
        OwnerCharacter->Jump();
        return;
    }

    // Buffer the jump — will fire on next ground contact
    bJumpBuffered     = true;
    JumpBufferCounter = JumpBufferDuration;
}

bool UJumpComponent::CanCoyoteJump() const
{
    if (!OwnerCharacter) return false;
    return !OwnerCharacter->GetCharacterMovement()->IsMovingOnGround()
        && CoyoteTimeCounter > 0.f
        && !bCoyoteJumpUsed;
}

void UJumpComponent::TickCoyoteTime(float DeltaTime)
{
    if (!OwnerCharacter) return;

    const bool bIsGrounded = OwnerCharacter->GetCharacterMovement()->IsMovingOnGround();

    if (bWasGrounded && !bIsGrounded)
    {
        // Just left the ground: start coyote window
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

void UJumpComponent::TickJumpBuffer(float DeltaTime)
{
    if (!bJumpBuffered || !OwnerCharacter) return;

    JumpBufferCounter -= DeltaTime;

    if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
    {
        OwnerCharacter->Jump();
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

void UJumpComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickCoyoteTime(DeltaTime);
    TickJumpBuffer(DeltaTime);
}