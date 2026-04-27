#include "WallJumpComponent.h"
#include "../PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UWallJumpComponent::UWallJumpComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWallJumpComponent::Initialize(APlayerCharacter* InOwner)
{
    OwnerCharacter = InOwner;
}

void UWallJumpComponent::ResetWallJumps()
{
    WallJumpCount = 0;
}

bool UWallJumpComponent::DetectWall(FHitResult& OutHit) const
{
    if (!OwnerCharacter) return false;

    const TArray<FVector> Directions = {
        OwnerCharacter->GetActorForwardVector(),
       -OwnerCharacter->GetActorForwardVector(),
        OwnerCharacter->GetActorRightVector(),
       -OwnerCharacter->GetActorRightVector()
    };

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    for (const FVector& Dir : Directions)
    {
        const FVector Start = OwnerCharacter->GetActorLocation();
        const FVector End   = Start + Dir * WallDetectionRange;

        if (OwnerCharacter->GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, Params))
        {
            // Ignore ceiling and floor surfaces
            if (FMath::Abs(OutHit.Normal.Z) < 0.3f)
                return true;
        }
    }
    return false;
}

bool UWallJumpComponent::TryWallJump()
{
    if (!OwnerCharacter)                                              return false;
    if (WallJumpCount >= MaxWallJumps)                               return false;
    if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())  return false;
    if (!bIsOnWall)                                                  return false;

    WallJumpCount++;

    FVector JumpVelocity = WallNormal * WallJumpLateralForce;
    JumpVelocity.Z       = WallJumpZVelocity;
    OwnerCharacter->GetCharacterMovement()->Velocity     = JumpVelocity;
    OwnerCharacter->GetCharacterMovement()->GravityScale = 1.f;

    bIsOnWall = false;
    return true;
}

void UWallJumpComponent::TickWallState(float DeltaTime)
{
    if (!OwnerCharacter) return;

    FHitResult Hit;
    const bool bWallDetected = DetectWall(Hit);
    const bool bIsAirborne   = !OwnerCharacter->GetCharacterMovement()->IsMovingOnGround();

    if (bWallDetected && bIsAirborne)
    {
        bIsOnWall  = true;
        WallNormal = Hit.Normal;
        OwnerCharacter->GetCharacterMovement()->GravityScale = WallGravityScale;
    }
    else
    {
        // Bug fix: original code set bIsOnWall = false then immediately checked !bIsOnWall (always true)
        bIsOnWall = false;
        OwnerCharacter->GetCharacterMovement()->GravityScale = 1.f;
    }
}

void UWallJumpComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickWallState(DeltaTime);
}