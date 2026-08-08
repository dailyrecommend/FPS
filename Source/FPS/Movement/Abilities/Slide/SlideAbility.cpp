#include "Movement/Abilities/Slide/SlideAbility.h"
#include "Weapons/Registry/WeaponRegistry.h"
#include "Weapons/Base/WeaponBase.h"
#include "Character/PlayerInputRouter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"

USlideAbility::USlideAbility()
{
    AbilityId = TEXT("Slide");
    Cooldown  = 0.f;
}

bool USlideAbility::CheckPreconditions(const FAbilityContext& /*Context*/) const
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) return false;

    return MoveComp->IsMovingOnGround()
        && Owner->GetVelocity().Size2D() > MinSpeed
        && !bIsActive;
}

EActivationResult USlideAbility::OnTryActivate(const FAbilityContext& Context)
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
        SlideDirection = (Forward * Context.MoveInput.Y + Right * Context.MoveInput.X).GetSafeNormal2D();
    else
        SlideDirection = Owner->GetActorForwardVector().GetSafeNormal2D();

    if (SlideDirection.IsNearlyZero())
        return EActivationResult::Failed_NotReady;

    MoveComp->Velocity                   = SlideDirection * BoostSpeed;
    MoveComp->GroundFriction             = 0.f;
    MoveComp->BrakingDecelerationWalking = 0.f;
    SmoothedRollInput                    = 0.f;

    // 캡슐 축소
    UCapsuleComponent* Capsule = Owner->GetCapsuleComponent();
    if (Capsule)
    {
        DefaultCapsuleHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
        Capsule->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight + CapsuleHeightOffset);
    }

    if (UObject* Effects = CameraEffects.GetObject())
    {
        HeightHandle = ICameraEffects::Execute_PushHeightOffset(Effects, HeightOffset, HeightInterp,    CameraPriority);
        RollHandle   = ICameraEffects::Execute_PushRollOffset  (Effects, 0.f,          RollInterpSpeed, CameraPriority);
    }

    UAnimMontage* Montage = nullptr;
    if (WeaponRegistry)
    {
        TScriptInterface<IWeapon> CurrentWeapon = WeaponRegistry->GetCurrentWeapon();
        if (UWeaponBase* WeaponBase = Cast<UWeaponBase>(CurrentWeapon.GetObject()))
            Montage = WeaponBase->GetSlideMontage();
    }
    PlayMontage(Montage);
    return EActivationResult::Success;
}

void USlideAbility::OnDeactivate()
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (MoveComp)
    {
        UCharacterMovementComponent* DefaultMove = MoveComp->GetClass()->GetDefaultObject<UCharacterMovementComponent>();
        MoveComp->GroundFriction             = DefaultMove ? DefaultMove->GroundFriction             : 8.f;
        MoveComp->BrakingDecelerationWalking = DefaultMove ? DefaultMove->BrakingDecelerationWalking : 2048.f;
    }

    // 캡슐 복원
    if (Owner)
    {
        UCapsuleComponent* Capsule = Owner->GetCapsuleComponent();
        if (Capsule)
            Capsule->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight);
    }

    if (UObject* Effects = CameraEffects.GetObject())
    {
        ICameraEffects::Execute_PopHeightOffset(Effects, HeightHandle);
        ICameraEffects::Execute_PopRollOffset  (Effects, RollHandle);
    }
    HeightHandle = 0;
    RollHandle   = 0;

    if (WeaponRegistry)
    {
        TScriptInterface<IWeapon> CurrentWeapon = WeaponRegistry->GetCurrentWeapon();
        if (UWeaponBase* WeaponBase = Cast<UWeaponBase>(CurrentWeapon.GetObject()))
            StopMontage(WeaponBase->GetSlideMontage());
    }
}

void USlideAbility::TickComponent(float DeltaTime, ELevelTick TickType,
                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsActive) return;

    const FVector2D MoveInput = InputRouter ? InputRouter->GetCurrentMoveInput() : FVector2D::ZeroVector;
    TickSlide(DeltaTime, MoveInput);
}

void USlideAbility::TickSlide(float DeltaTime, const FVector2D& MoveInput)
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) { Deactivate_Implementation(); return; }

    if (Owner->GetVelocity().Size2D() < MinSpeed) { Deactivate_Implementation(); return; }

    const FVector SlideRight   = FVector::CrossProduct(FVector::UpVector, SlideDirection).GetSafeNormal();
    const FVector LateralInput = SlideRight * MoveInput.X * LateralControl * DeltaTime;
    FVector       TargetVel    = SlideDirection * BoostSpeed + LateralInput;
    TargetVel.Z                = MoveComp->Velocity.Z;
    MoveComp->Velocity         = TargetVel;

    // 입력을 한 번 보간(ease-in) → CameraEffects에서 한 번 더 보간(ease-out) = ease-in-out
    SmoothedRollInput = FMath::FInterpTo(SmoothedRollInput, MoveInput.X, DeltaTime, RollInputSmoothSpeed);

    const float TargetRoll = SmoothedRollInput * MaxRollDegrees;
    if (UObject* Effects = CameraEffects.GetObject())
        ICameraEffects::Execute_UpdateRollOffset(Effects, RollHandle, TargetRoll);
}