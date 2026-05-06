#include "Movement/Abilities/Glissando/GlissandoAbility.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

UGlissandoAbility::UGlissandoAbility()
{
    AbilityId = TEXT("Glissando");
    Cooldown  = 0.f;
}

void UGlissandoAbility::InjectCamera(UCameraComponent* InCamera)
{
    Camera = InCamera;
}

bool UGlissandoAbility::CheckPreconditions(const FAbilityContext& /*Context*/) const
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) return false;

    return MoveComp->IsMovingOnGround()
        && Owner->GetVelocity().Size2D() > MinSpeed
        && !bIsActive;
}

EActivationResult UGlissandoAbility::OnTryActivate(const FAbilityContext& Context)
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) return EActivationResult::Failed_NoOwner;

    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC) return EActivationResult::Failed_NotReady;

    const FRotator        CamRot = PC->GetControlRotation();
    const FRotationMatrix YawMatrix(FRotator(0.f, CamRot.Yaw, 0.f));
    const FVector         Forward = YawMatrix.GetUnitAxis(EAxis::X);
    const FVector         Right   = YawMatrix.GetUnitAxis(EAxis::Y);

    if (!Context.MoveInput.IsNearlyZero())
        GlissandoDirection = (Forward * Context.MoveInput.Y + Right * Context.MoveInput.X).GetSafeNormal2D();
    else
        GlissandoDirection = Owner->GetActorForwardVector().GetSafeNormal2D();

    if (GlissandoDirection.IsNearlyZero())
        return EActivationResult::Failed_NotReady;

    MoveComp->Velocity                   = GlissandoDirection * BoostSpeed;
    MoveComp->GroundFriction             = 0.f;
    MoveComp->BrakingDecelerationWalking = 0.f;

    if (UCameraComponent* Cam = Camera.Get())
        Cam->SetRelativeLocation(FVector(0.f, 0.f, Owner->BaseEyeHeight + CameraHeight));

    LastMoveInput = Context.MoveInput;
    return EActivationResult::Success;
}

void UGlissandoAbility::OnDeactivate()
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) return;

    UCharacterMovementComponent* DefaultMove = MoveComp->GetClass()->GetDefaultObject<UCharacterMovementComponent>();
    MoveComp->GroundFriction             = DefaultMove ? DefaultMove->GroundFriction             : 8.f;
    MoveComp->BrakingDecelerationWalking = DefaultMove ? DefaultMove->BrakingDecelerationWalking : 2048.f;

    if (UCameraComponent* Cam = Camera.Get())
        Cam->SetRelativeLocation(FVector(0.f, 0.f, Owner->BaseEyeHeight));

    ResetCameraRoll();
    CurrentCameraRoll = 0.f;
}

void UGlissandoAbility::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsActive)
        TickGlissando(DeltaTime, LastMoveInput);

    LastMoveInput = FVector2D::ZeroVector;
}

void UGlissandoAbility::TickGlissando(float DeltaTime, const FVector2D& MoveInput)
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp)
    {
        Deactivate_Implementation();
        return;
    }

    if (Owner->GetVelocity().Size2D() < MinSpeed)
    {
        Deactivate_Implementation();
        return;
    }

    FVector GlissandoRight = FVector::CrossProduct(FVector::UpVector, GlissandoDirection).GetSafeNormal();
    FVector LateralInput   = GlissandoRight * MoveInput.X * LateralControl * DeltaTime;
    FVector TargetVelocity = GlissandoDirection * BoostSpeed + LateralInput;
    TargetVelocity.Z       = MoveComp->Velocity.Z;
    MoveComp->Velocity     = TargetVelocity;

    const float TargetRoll = MoveInput.X * CameraRoll;
    CurrentCameraRoll = FMath::FInterpTo(CurrentCameraRoll, TargetRoll, DeltaTime, CameraRollInterpSpeed);
    ApplyCameraRoll(CurrentCameraRoll);
}

void UGlissandoAbility::ApplyCameraRoll(float Roll)
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return;

    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC) return;

    FRotator ControlRot = PC->GetControlRotation();
    ControlRot.Roll     = Roll;
    PC->SetControlRotation(ControlRot);
}

void UGlissandoAbility::ResetCameraRoll()
{
    ApplyCameraRoll(0.f);
}