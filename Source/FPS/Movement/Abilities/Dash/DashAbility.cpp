#include "Movement/Abilities/Dash/DashAbility.h"
#include "Movement/Registry/AbilityRegistry.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

UDashAbility::UDashAbility()
{
    AbilityId   = TEXT("Dash");
    Cooldown    = 0.f;
    DashCharges = MaxDashCharges;
}

void UDashAbility::BeginPlay()
{
    Super::BeginPlay();
    DashCharges = MaxDashCharges;
}

bool UDashAbility::CheckPreconditions(const FAbilityContext& /*Context*/) const
{
    return DashCharges > 0 && !bIsActive && !bDashOnCooldown;
}

EActivationResult UDashAbility::OnTryActivate(const FAbilityContext& Context)
{
    ACharacter* Owner = GetOwnerSafe();
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!Owner || !MoveComp) return EActivationResult::Failed_NoOwner;

    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC) return EActivationResult::Failed_NotReady;

    RequestSlamCancelIfActive();

    const FRotator        CamRot = PC->GetControlRotation();
    const FRotationMatrix YawMatrix(FRotator(0.f, CamRot.Yaw, 0.f));
    const FVector         Forward = YawMatrix.GetUnitAxis(EAxis::X);
    const FVector         Right   = YawMatrix.GetUnitAxis(EAxis::Y);

    if (!Context.MoveInput.IsNearlyZero())
        DashDirection = (Forward * Context.MoveInput.Y + Right * Context.MoveInput.X).GetSafeNormal2D();
    else
        DashDirection = Forward.GetSafeNormal2D();

    if (DashDirection.IsNearlyZero())
        return EActivationResult::Failed_NotReady;

    MoveComp->Velocity = DashDirection * DashSpeed;

    DashElapsed       = 0.f;
    bDashChargeDelay  = true;
    DashDelayTimer    = 0.f;
    bDashOnCooldown   = true;
    DashCooldownTimer = 0.f;
    DashCharges = FMath::Max(0, DashCharges - 1);
    OnDashChargesChanged.Broadcast(DashCharges, MaxDashCharges);

    PlayMontage(DashMontage);
    return EActivationResult::Success;
}

void UDashAbility::OnDeactivate()
{
    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp) return;

    const bool bIsGrounded = MoveComp->IsMovingOnGround();
    MoveComp->Velocity = bIsGrounded ? FVector::ZeroVector : (DashDirection * DashAirMomentumSpeed);
}

void UDashAbility::TickComponent(float DeltaTime, ELevelTick TickType,
                                 FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickDash(DeltaTime);
    TickCooldown(DeltaTime);
    TickChargeRecovery(DeltaTime);
}

void UDashAbility::TickDash(float DeltaTime)
{
    if (!bIsActive) return;

    UCharacterMovementComponent* MoveComp = GetMoveComp();
    if (!MoveComp)
    {
        Deactivate_Implementation();
        return;
    }

    DashElapsed += DeltaTime;
    MoveComp->Velocity = DashDirection * DashSpeed;

    if (DashElapsed >= DashDuration)
        Deactivate_Implementation();
}

void UDashAbility::TickCooldown(float DeltaTime)
{
    if (!bDashOnCooldown) return;

    DashCooldownTimer += DeltaTime;
    if (DashCooldownTimer >= DashCooldown)
        bDashOnCooldown = false;
}

void UDashAbility::TickChargeRecovery(float DeltaTime)
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

    if (DashCharges < MaxDashCharges)
    {
        DashChargeTimer += DeltaTime;
        if (DashChargeTimer >= DashChargeInterval)
        {
            DashChargeTimer = 0.f;
            AddDashCharge();
        }
    }
}

void UDashAbility::AddDashCharge()
{
    DashCharges = FMath::Min(DashCharges + 1, MaxDashCharges);
    OnDashChargesChanged.Broadcast(DashCharges, MaxDashCharges);
}

void UDashAbility::AddDashChargeImmediate()
{
    AddDashCharge();
    bDashChargeDelay = false;
    DashChargeTimer  = 0.f;
}

void UDashAbility::RequestSlamCancelIfActive() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UAbilityRegistry* Registry = Owner->FindComponentByClass<UAbilityRegistry>();
    if (!Registry) return;

    Registry->Cancel(SlamAbilityIdToCancel);
}