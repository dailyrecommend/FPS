#include "DashComponent.h"
#include "../PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

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

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (PC)
    {
        FRotator CamRot = PC->GetControlRotation();
        FRotationMatrix RotMatrix(FRotator(0, CamRot.Yaw, 0));

        FVector Forward = RotMatrix.GetUnitAxis(EAxis::X);
        FVector Right   = RotMatrix.GetUnitAxis(EAxis::Y);
        

        if (!LastMoveInput.IsNearlyZero())
        {
            DashDirection = Forward * LastMoveInput.Y + Right * LastMoveInput.X;
            DashDirection.Z = 0.f;
            DashDirection.Normalize();
        }
        else
        {
            DashDirection = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X);
            DashDirection.Normalize();
        }
    }

    OwnerCharacter->GetCharacterMovement()->Velocity = DashDirection * DashSpeed;
    bDashChargeDelay = true;
    DashDelayTimer   = 0.f;
}

void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickDash(DeltaTime);
    LastMoveInput = FVector2D::ZeroVector;
}

void UDashComponent::TickDash(float DeltaTime)
{
    if (bIsDashing)
    {
        DashElapsed += DeltaTime;
        OwnerCharacter->GetCharacterMovement()->Velocity = DashDirection * DashSpeed;

        if (DashElapsed >= DashDuration)
        {
            bIsDashing = false;

            if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
                OwnerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
            else
                OwnerCharacter->GetCharacterMovement()->Velocity = DashDirection * DashAirMomentumSpeed;
        }
    }

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