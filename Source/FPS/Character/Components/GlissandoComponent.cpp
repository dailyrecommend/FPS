#include "GlissandoComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

UGlissandoComponent::UGlissandoComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGlissandoComponent::Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera)
{
    OwnerCharacter = InOwner;
    Camera         = InCamera;
}

void UGlissandoComponent::OnMoveInput(FVector2D Input)
{
    CurrentMoveInput = Input;
}

bool UGlissandoComponent::CanGlissando() const
{
    if (!OwnerCharacter) return false;
    return OwnerCharacter->GetCharacterMovement()->IsMovingOnGround()
        && OwnerCharacter->GetVelocity().Size2D() > GlissandoMinSpeed
        && !bIsGlissando;
}

void UGlissandoComponent::StartGlissando()
{
    if (!OwnerCharacter || !Camera) return;

    bIsGlissando = true;

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (PC)
    {
        FRotator CamRot = PC->GetControlRotation();
        FRotationMatrix RotMatrix(FRotator(0, CamRot.Yaw, 0));

        FVector Forward = RotMatrix.GetUnitAxis(EAxis::X);
        FVector Right   = RotMatrix.GetUnitAxis(EAxis::Y);

        if (!CurrentMoveInput.IsNearlyZero())
        {
            GlissandoDirection = Forward * CurrentMoveInput.Y + Right * CurrentMoveInput.X;
            GlissandoDirection.Z = 0.f;
            GlissandoDirection.Normalize();
        }
        else
        {
            GlissandoDirection = OwnerCharacter->GetActorForwardVector();
            GlissandoDirection.Z = 0.f;
            GlissandoDirection.Normalize();
        }
    }
    

    OwnerCharacter->GetCharacterMovement()->Velocity               = GlissandoDirection * GlissandoBoostSpeed;
    OwnerCharacter->GetCharacterMovement()->GroundFriction         = 0.f;
    OwnerCharacter->GetCharacterMovement()->BrakingDecelerationWalking = 0.f;
    Camera->SetRelativeLocation(FVector(0.f, 0.f, OwnerCharacter->GetDefaultCameraHeight() + GlissandoCameraHeight));
}

void UGlissandoComponent::EndGlissando()
{
    if (!bIsGlissando || !OwnerCharacter || !Camera) return;
    bIsGlissando = false;

    OwnerCharacter->GetCharacterMovement()->GroundFriction             = OwnerCharacter->GetDefaultGroundFriction();
    OwnerCharacter->GetCharacterMovement()->BrakingDecelerationWalking = OwnerCharacter->GetDefaultBrakingDeceleration();
    Camera->SetRelativeLocation(FVector(0.f, 0.f, OwnerCharacter->GetDefaultCameraHeight()));
    CurrentCameraRoll = 0.f;

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (PC)
    {
        FRotator ControlRot = PC->GetControlRotation();
        ControlRot.Roll     = 0.f;
        PC->SetControlRotation(ControlRot);
    }
}

void UGlissandoComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickGlissando(DeltaTime);
    CurrentMoveInput = FVector2D::ZeroVector;
}
void UGlissandoComponent::TickGlissando(float DeltaTime)
{
    if (!bIsGlissando || !OwnerCharacter) return;

    float CurrentSpeed = OwnerCharacter->GetVelocity().Size2D();

    if (CurrentSpeed < GlissandoMinSpeed)
    {
        EndGlissando();
        return;
    }

    FVector GlissandoRight = FVector::CrossProduct(FVector::UpVector, GlissandoDirection);
    GlissandoRight.Normalize();

    FVector LateralInput   = GlissandoRight * CurrentMoveInput.X * GlissandoLateralControl * DeltaTime;
    FVector TargetVelocity = GlissandoDirection * GlissandoBoostSpeed + LateralInput;
    TargetVelocity.Z       = OwnerCharacter->GetCharacterMovement()->Velocity.Z;
    OwnerCharacter->GetCharacterMovement()->Velocity = TargetVelocity;

    float TargetRoll  = CurrentMoveInput.X * GlissandoCameraRoll;
    CurrentCameraRoll = FMath::FInterpTo(CurrentCameraRoll, TargetRoll, DeltaTime, GlissandoCameraRollInterpSpeed);

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (PC)
    {
        FRotator ControlRot = PC->GetControlRotation();
        ControlRot.Roll     = CurrentCameraRoll;
        PC->SetControlRotation(ControlRot);
    }
}