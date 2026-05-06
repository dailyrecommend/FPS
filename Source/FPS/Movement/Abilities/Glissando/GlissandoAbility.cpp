#include "Movement/Abilities/Glissando/GlissandoAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

UGlissandoAbility::UGlissandoAbility()
{
    AbilityId = TEXT("Glissando");
    Cooldown  = 0.f;
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

    if (UObject* Effects = CameraEffects.GetObject())
    {
        HeightHandle = ICameraEffects::Execute_PushHeightOffset(Effects, HeightOffset, HeightInterp,    CameraPriority);
        RollHandle   = ICameraEffects::Execute_PushRollOffset  (Effects, 0.f,         RollInterpSpeed, CameraPriority);
    }

    LastMoveInput = Context.MoveInput;
    return EActivationResult::Success;
}

void UGlissandoAbility::OnDeactivate()
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (MoveComp)
    {
        UCharacterMovementComponent* DefaultMove = MoveComp->GetClass()->GetDefaultObject<UCharacterMovementComponent>();
        MoveComp->GroundFriction             = DefaultMove ? DefaultMove->GroundFriction             : 8.f;
        MoveComp->BrakingDecelerationWalking = DefaultMove ? DefaultMove->BrakingDecelerationWalking : 2048.f;
    }

    if (UObject* Effects = CameraEffects.GetObject())
    {
        ICameraEffects::Execute_PopHeightOffset(Effects, HeightHandle);
        ICameraEffects::Execute_PopRollOffset  (Effects, RollHandle);
    }
    HeightHandle = 0;
    RollHandle   = 0;
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
    if (!Owner || !MoveComp) { Deactivate_Implementation(); return; }

    if (Owner->GetVelocity().Size2D() < MinSpeed) { Deactivate_Implementation(); return; }

    const FVector GlissandoRight  = FVector::CrossProduct(FVector::UpVector, GlissandoDirection).GetSafeNormal();
    const FVector LateralInput    = GlissandoRight * MoveInput.X * LateralControl * DeltaTime;
    FVector       TargetVelocity  = GlissandoDirection * BoostSpeed + LateralInput;
    TargetVelocity.Z              = MoveComp->Velocity.Z;
    MoveComp->Velocity            = TargetVelocity;

    const float TargetRoll = MoveInput.X * MaxRollDegrees;
    if (UObject* Effects = CameraEffects.GetObject())
        ICameraEffects::Execute_UpdateRollOffset(Effects, RollHandle, TargetRoll);
}