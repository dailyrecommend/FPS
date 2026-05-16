#include "Weapons/Implementations/SwordSkill/SwordSkill.h"
#include "Weapons/Implementations/Sword/SwordWeapon.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Core/Subsystems/TimeDilationSubsystem.h"
#include "Core/Data/TimeDilationRequest.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/App.h"

USwordSkill::USwordSkill()
{
}

bool USwordSkill::OnStartHold()
{
    if (bIsDashing || bIsStunned) return false;

    HoldElapsed = 0.f;
    RequestTimeDilation();
    PlayMontage(HoldMontage);
    OnSwordSkillStarted.Broadcast();
    return true;
}

void USwordSkill::OnEndHold()
{
    ReleaseTimeDilation();
    PerformDash();
}

void USwordSkill::OnCancel()
{
    HoldElapsed = 0.f;
    ReleaseTimeDilation();
    OnSwordSkillCancelled.Broadcast();
}

void USwordSkill::EndPlay(EEndPlayReason::Type Reason)
{
    ReleaseTimeDilation();
    RestorePawnCollision();
    Super::EndPlay(Reason);
}

void USwordSkill::PerformDash()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !GetCameraSafe()) return;

    DashStart       = Owner->GetActorLocation();
    DashDestination = CalculateDestination();

    HitActorsAlongPath(DashStart, DashDestination);
    DisablePawnCollision();

    bIsDashing  = true;
    DashElapsed = 0.f;
    StartCooldown(CooldownDuration);

    PlayMontage(DashMontage);
}

FVector USwordSkill::CalculateDestination() const
{
    ACharacter* Owner     = GetOwnerSafe();
    UCameraComponent* Cam = GetCameraSafe();
    if (!Owner || !Cam || !Owner->GetWorld()) return FVector::ZeroVector;

    UCapsuleComponent* Capsule = Owner->GetCapsuleComponent();
    if (!Capsule) return Owner->GetActorLocation();

    const FVector Start     = Owner->GetActorLocation();
    const FVector Direction = Cam->GetForwardVector().GetSafeNormal();
    const FVector End       = Start + Direction * Distance;

    const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
    const float CapsuleHalf   = Capsule->GetScaledCapsuleHalfHeight();

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

    const bool bHit = Owner->GetWorld()->SweepSingleByObjectType(
        Hit, Start, End, FQuat::Identity, ObjectParams,
        FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalf), Params);

    return bHit ? Hit.Location + Hit.Normal * (CapsuleRadius + 5.f) : End;
}

void USwordSkill::HitActorsAlongPath(const FVector& Start, const FVector& End)
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    UCapsuleComponent* Capsule = Owner->GetCapsuleComponent();
    if (!Capsule) return;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    TArray<FHitResult> Hits;
    Owner->GetWorld()->SweepMultiByChannel(
        Hits, Start, End, FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeCapsule(
            Capsule->GetScaledCapsuleRadius(),
            Capsule->GetScaledCapsuleHalfHeight()),
        Params);

    TSet<AActor*>  HitActors;
    AController*   Instigator = Owner->GetController();

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActors.Contains(HitActor)) continue;
        HitActors.Add(HitActor);

        FHitResultBuilder Builder;
        Builder
            .From(Instigator)
            .Target(HitActor)
            .At(Hit.ImpactPoint, Hit.ImpactNormal)
            .WithDamage(Damage)
            .OfDamageType(EWeaponDamageType::Sword)
            .OfHitType(EHitType::Iajutsu);

        OnSwordSkillHit.Broadcast(Builder.Build());
        Builder.Apply();
    }
}

void USwordSkill::RequestTimeDilation()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    UTimeDilationSubsystem* TimeSys = Owner->GetWorld()->GetSubsystem<UTimeDilationSubsystem>();
    if (!TimeSys) return;

    if (TimeDilationHandle != 0) { TimeSys->PopRequest(TimeDilationHandle); TimeDilationHandle = 0; }

    FTimeDilationRequest Request;
    Request.WorldDilation = SlowWorldDilation;
    Request.Mode          = ETimeScaleMode::Full;
    Request.BlendIn       = DilationBlendIn;
    Request.BlendOut      = DilationBlendOut;
    Request.Priority      = DilationPriority;
    Request.Requester     = Owner;

    TimeDilationHandle = TimeSys->PushRequest(Request);
}

void USwordSkill::ReleaseTimeDilation()
{
    if (TimeDilationHandle == 0) return;

    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) { TimeDilationHandle = 0; return; }

    if (UTimeDilationSubsystem* TimeSys = Owner->GetWorld()->GetSubsystem<UTimeDilationSubsystem>())
        TimeSys->PopRequest(TimeDilationHandle);

    TimeDilationHandle = 0;
}

void USwordSkill::DisablePawnCollision()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return;
    if (UCapsuleComponent* Cap = Owner->GetCapsuleComponent())
        Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void USwordSkill::RestorePawnCollision()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return;
    if (UCapsuleComponent* Cap = Owner->GetCapsuleComponent())
        Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void USwordSkill::TickHold(float UnscaledDelta)
{
    if (!bIsActive) return;
    HoldElapsed += UnscaledDelta;
    if (HoldElapsed >= HoldMaxDuration) Cancel();
}

void USwordSkill::TickDash(float UnscaledDelta)
{
    if (!bIsDashing) return;

    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) { bIsDashing = false; return; }

    DashElapsed += UnscaledDelta;

    const float Alpha    = FMath::Clamp(DashElapsed / FMath::Max(DashDuration, KINDA_SMALL_NUMBER), 0.f, 1.f);
    const FVector NewLoc = FMath::Lerp(DashStart, DashDestination, Alpha);
    Owner->SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);

    if (Alpha >= 1.f)
    {
        bIsDashing = false;

        if (UCharacterMovementComponent* MoveComp = Owner->GetCharacterMovement())
            MoveComp->Velocity = FVector::ZeroVector;

        RestorePawnCollision();
        bIsStunned  = true;
        StunElapsed = 0.f;
        OnSwordSkillEnded.Broadcast();
    }
}

void USwordSkill::TickStun(float UnscaledDelta)
{
    if (!bIsStunned) return;
    StunElapsed += UnscaledDelta;
    if (StunElapsed >= StunDuration) bIsStunned = false;
}

void USwordSkill::TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const float UnscaledDelta = FApp::GetDeltaTime();
    TickHold(UnscaledDelta);
    TickDash(UnscaledDelta);
    TickStun(UnscaledDelta);
    TickCooldown(UnscaledDelta);
}