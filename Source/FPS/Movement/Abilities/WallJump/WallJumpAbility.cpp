#include "Movement/Abilities/WallJump/WallJumpAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UWallJumpAbility::UWallJumpAbility()
{
    AbilityId = TEXT("WallJump");
    Cooldown  = 0.f;
}

void UWallJumpAbility::ResetWallJumps()
{
    WallJumpCount = 0;
}

bool UWallJumpAbility::CheckPreconditions(const FAbilityContext& /*Context*/) const
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return false;
    return bIsOnWall && !MoveComp->IsMovingOnGround() && WallJumpCount < MaxWallJumps;
}

EActivationResult UWallJumpAbility::OnTryActivate(const FAbilityContext& /*Context*/)
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return EActivationResult::Failed_NoOwner;

    WallJumpCount++;

    FVector JumpVelocity = WallNormal * WallJumpLateralForce;
    JumpVelocity.Z       = WallJumpZVelocity;
    MoveComp->Velocity     = JumpVelocity;
    MoveComp->GravityScale = 1.f;

    bIsOnWall = false;

    // Wall jump is instantaneous — deactivate next frame so IsActive() stays consistent
    Deactivate_Implementation();
    return EActivationResult::Success;
}

bool UWallJumpAbility::DetectWall(FHitResult& OutHit) const
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return false;

    const TArray<FVector> Directions = {
        Owner->GetActorForwardVector(),
       -Owner->GetActorForwardVector(),
        Owner->GetActorRightVector(),
       -Owner->GetActorRightVector()
    };

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    const FVector Start = Owner->GetActorLocation();

    for (const FVector& Dir : Directions)
    {
        const FVector End = Start + Dir * WallDetectionRange;
        if (Owner->GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, Params))
        {
            if (FMath::Abs(OutHit.Normal.Z) < 0.3f)
                return true;
        }
    }
    return false;
}

void UWallJumpAbility::TickWallState()
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return;

    if (MoveComp->IsMovingOnGround())
    {
        if (bIsOnWall) MoveComp->GravityScale = 1.f;
        bIsOnWall = false;
        WallJumpCount = 0;
        return;
    }

    FHitResult Hit;
    const bool bWallDetected = DetectWall(Hit);

    if (bWallDetected)
    {
        bIsOnWall = true;
        WallNormal = Hit.Normal;
        MoveComp->GravityScale = WallGravityScale;
    }
    else
    {
        if (bIsOnWall) MoveComp->GravityScale = 1.f;
        bIsOnWall = false;
    }
}

void UWallJumpAbility::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickWallState();
}