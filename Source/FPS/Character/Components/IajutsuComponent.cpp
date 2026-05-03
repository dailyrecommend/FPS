#include "IajutsuComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "SwordComponent.h"
#include "TimeScaleComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Combat/DamageableInterface.h"

UIajutsuComponent::UIajutsuComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UIajutsuComponent::Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera, USwordComponent* InSword)
{
    OwnerCharacter = InOwner;
    Camera         = InCamera;
    Sword          = InSword;
}

bool UIajutsuComponent::CanIajutsu() const
{
    return !bIsHolding && !bIsDashing && CooldownRemaining <= 0.f;
}

void UIajutsuComponent::StartHold()
{
    if (!CanIajutsu() || !OwnerCharacter) return;

    bIsHolding  = true;
    HoldElapsed = 0.f;

    ApplyTimeScale();
    PlayMontage(HoldMontage);
    OnIajutsuStarted.Broadcast();
}

void UIajutsuComponent::EndHold()
{
    if (!bIsHolding) return;

    bIsHolding = false;
    ClearTimeScale();
    PerformDash();
}

void UIajutsuComponent::Cancel()
{
    bIsHolding  = false;
    HoldElapsed = 0.f;

    ClearTimeScale();

    // No cooldown on cancel
    OnIajutsuCancelled.Broadcast();
}

void UIajutsuComponent::PerformDash()
{
    DashStart       = OwnerCharacter->GetActorLocation();
    DashDestination = CalculateDestination();

    HitActorsAlongPath(DashStart, DashDestination);

    // Ignore pawn collision during dash so we pass through enemies
    OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    bIsDashing  = true;
    DashElapsed = 0.f;

    CooldownRemaining = IajutsuCooldown;
    PlayMontage(DashMontage);
}

FVector UIajutsuComponent::CalculateDestination() const
{
    const FVector Start     = OwnerCharacter->GetActorLocation();
    const FVector Direction = Camera->GetForwardVector().GetSafeNormal();
    const FVector End       = Start + Direction * IajutsuDistance;

    const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float CapsuleHalf   = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    // 트레이스 채널 대신, 오브젝트 타입 자체를 질의(Query)하도록 변경
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic); // 지형, 벽 등만 검사

    // SweepSingleByChannel -> SweepSingleByObjectType 으로 변경
    const bool bHitObstacle = OwnerCharacter->GetWorld()->SweepSingleByObjectType(
        Hit, Start, End, FQuat::Identity,
        ObjectParams,
        FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalf),
        Params
    );

    if (bHitObstacle)
    {
        const FVector SafeLocation = Hit.Location + Hit.Normal * (CapsuleRadius + 5.f);
        return SafeLocation;
    }

    return End;
}

void UIajutsuComponent::HitActorsAlongPath(const FVector& Start, const FVector& End)
{
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    TArray<FHitResult> Hits;
    OwnerCharacter->GetWorld()->SweepMultiByChannel(
        Hits, Start, End, FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeCapsule(
            OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
            OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
        ),
        Params
    );

    TSet<AActor*> HitActors;
    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActors.Contains(HitActor)) continue;
        HitActors.Add(HitActor);
        BroadcastHit(HitActor, Hit.ImpactPoint, Hit.ImpactNormal);
    }
}

void UIajutsuComponent::BroadcastHit(AActor* HitActor, const FVector& Location, const FVector& Normal)
{
    FWeaponHitResult WeaponHit;
    WeaponHit.HitActor    = HitActor;
    WeaponHit.HitLocation = Location;
    WeaponHit.HitNormal   = Normal;
    WeaponHit.Damage      = IajutsuDamage;
    WeaponHit.DamageType  = EWeaponDamageType::Sword;
    WeaponHit.HitType     = EHitType::Iajutsu;
    WeaponHit.bIsCritical = false;
    WeaponHit.Instigator  = OwnerCharacter->GetController();

    OnHit.Broadcast(WeaponHit);

    if (HitActor->Implements<UDamageable>())
        IDamageable::Execute_OnWeaponHit(HitActor, WeaponHit);
}

void UIajutsuComponent::ApplyTimeScale()
{
    FTimeScaleParams Params;
    Params.Mode          = ETimeScaleMode::Full;
    Params.WorldDilation = SlowWorldDilation;
    Params.Duration      = 0.f;
    Params.BlendIn       = 0.1f;
    Params.BlendOut      = 0.2f;

    OwnerCharacter->GetTimeScaleComponent()->ApplyTimeScale(Params);
}

void UIajutsuComponent::ClearTimeScale()
{
    OwnerCharacter->GetTimeScaleComponent()->ClearTimeScale(0.f);
}

void UIajutsuComponent::TickHold(float DeltaTime)
{
    if (!bIsHolding) return;

    // Use unscaled delta so hold timer isn't affected by the slow itself
    HoldElapsed += FApp::GetDeltaTime();

    if (HoldElapsed >= HoldMaxDuration)
        Cancel();
}

void UIajutsuComponent::TickDash(float DeltaTime)
{
    if (!bIsDashing) return;

    DashElapsed += FApp::GetDeltaTime();

    const float Alpha    = FMath::Clamp(DashElapsed / IajutsuDashDuration, 0.f, 1.f);
    const FVector NewLoc = FMath::Lerp(DashStart, DashDestination, Alpha);
    OwnerCharacter->SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);

    if (Alpha >= 1.f)
    {
        bIsDashing = false;

        // Reset all velocity after dash
        OwnerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;

        // Restore pawn collision
        OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

        // Start post-dash stun
        bIsStunned  = true;
        StunElapsed = 0.f;

        OnIajutsuEnded.Broadcast();
    }
}

void UIajutsuComponent::TickStun(float DeltaTime)
{
    if (!bIsStunned) return;

    StunElapsed += FApp::GetDeltaTime();
    if (StunElapsed >= IajutsuStunDuration)
        bIsStunned = false;
}

void UIajutsuComponent::TickCooldown(float DeltaTime)
{
    if (CooldownRemaining > 0.f)
        CooldownRemaining = FMath::Max(0.f, CooldownRemaining - FApp::GetDeltaTime());
}

void UIajutsuComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickHold(DeltaTime);
    TickDash(DeltaTime);
    TickStun(DeltaTime);
    TickCooldown(DeltaTime);
}

void UIajutsuComponent::PlayMontage(UAnimMontage* Montage)
{
    if (!Montage || !OwnerCharacter) return;

    UAnimInstance* Anim = OwnerCharacter->GetArmsMesh()->GetAnimInstance();
    if (Anim) Anim->Montage_Play(Montage, 1.f);
}