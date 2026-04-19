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

    // 캐릭터 주변 4방향으로 벽 감지
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
        FVector Start = OwnerCharacter->GetActorLocation();
        FVector End   = Start + Dir * WallDetectionRange;

        if (OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
            OutHit, Start, End, ECC_WorldStatic, Params))
        {
            // 천장이나 바닥은 제외
            if (FMath::Abs(OutHit.Normal.Z) < 0.3f)
                return true;
        }
    }
    return false;
}

bool UWallJumpComponent::TryWallJump()
{
    if (!OwnerCharacter) return false;
    if (WallJumpCount >= MaxWallJumps) return false;
    if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround()) return false;
    if (!bIsOnWall) return false;

    WallJumpCount++;

    // 벽 법선 방향으로 튕겨나감
    FVector JumpVelocity = WallNormal * WallJumpLateralForce;
    JumpVelocity.Z       = WallJumpZVelocity;
    OwnerCharacter->GetCharacterMovement()->Velocity = JumpVelocity;

    // 중력 원래대로 복구
    OwnerCharacter->GetCharacterMovement()->GravityScale = 1.f;
    bIsOnWall = false;

    return true;
}

void UWallJumpComponent::ApplyWallGravity(float DeltaTime)
{
    if (!OwnerCharacter) return;

    FHitResult Hit;
    bool bWallDetected = DetectWall(Hit);

    if (bWallDetected && !OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
    {
        bIsOnWall  = true;
        WallNormal = Hit.Normal;
        OwnerCharacter->GetCharacterMovement()->GravityScale = WallGravityScale;
    }
    else
    {
        bIsOnWall = false;
        // 땅에 있거나 벽이 없으면 중력 복구
        if (!bIsOnWall)
            OwnerCharacter->GetCharacterMovement()->GravityScale = 1.f;
    }
}

void UWallJumpComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    ApplyWallGravity(DeltaTime);
}