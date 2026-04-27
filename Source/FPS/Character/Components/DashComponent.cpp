#include "DashComponent.h"
#include "../PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UDashComponent::UDashComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UDashComponent::BeginPlay()
{
    Super::BeginPlay();
    DashCharges = MaxDashCharges;
}

void UDashComponent::Initialize(APlayerCharacter* InOwner)
{
    OwnerCharacter = InOwner;
}

void UDashComponent::TryDash()
{
    if (DashCharges <= 0 || bIsDashing || !OwnerCharacter) return;
    PerformDash();
}

void UDashComponent::PerformDash()
{
    bIsDashing  = true;
    DashElapsed = 0.f;
    DashCharges--;

    const APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PC) return;

    const FRotator          CamRot    = PC->GetControlRotation();
    const FRotationMatrix   RotMatrix = FRotationMatrix(FRotator(0, CamRot.Yaw, 0));
    const FVector           Forward   = RotMatrix.GetUnitAxis(EAxis::X);
    const FVector           Right     = RotMatrix.GetUnitAxis(EAxis::Y);

    if (!LastMoveInput.IsNearlyZero())
    {
        DashDirection   = (Forward * LastMoveInput.Y + Right * LastMoveInput.X).GetSafeNormal2D();
    }
    else
    {
        DashDirection   = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X).GetSafeNormal2D();
    }

    OwnerCharacter->GetCharacterMovement()->Velocity = DashDirection * DashSpeed;
    bDashChargeDelay = true;
    DashDelayTimer   = 0.f;
}

void UDashComponent::TickDash(float DeltaTime)
{
    if (!bIsDashing) return;

    DashElapsed += DeltaTime;
    OwnerCharacter->GetCharacterMovement()->Velocity = DashDirection * DashSpeed;

    if (DashElapsed >= DashDuration)
    {
        bIsDashing = false;

        const bool bIsGrounded = OwnerCharacter->GetCharacterMovement()->IsMovingOnGround();
        OwnerCharacter->GetCharacterMovement()->Velocity = bIsGrounded
            ? FVector::ZeroVector
            : DashDirection * DashAirMomentumSpeed;
    }
}

void UDashComponent::TickChargeRecovery(float DeltaTime)
{
    if (bDashChargeDelay)
    {
        DashDelayTimer += DeltaTime;
        if (DashDelayTimer >= DashChargeDelay)
        {
            bDashChargeDelay = false;
            DashChargeTimer  = 0.f;
        }
        return;
    }

    if (DashCharges < MaxDashCharges && OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
    {
        DashChargeTimer += DeltaTime;
        if (DashChargeTimer >= DashChargeInterval)
        {
            DashChargeTimer = 0.f;
            AddDashCharge();
        }
    }
}

void UDashComponent::AddDashCharge()
{
    DashCharges = FMath::Min(DashCharges + 1, MaxDashCharges);
}

void UDashComponent::AddDashChargeImmediate()
{
    AddDashCharge();
    bDashChargeDelay = false;
    DashChargeTimer  = 0.f;
}

void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickDash(DeltaTime);
    TickChargeRecovery(DeltaTime);
    LastMoveInput = FVector2D::ZeroVector;
}