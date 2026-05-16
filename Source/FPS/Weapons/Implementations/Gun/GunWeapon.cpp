#include "Weapons/Implementations/Gun/GunWeapon.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"

UGunWeapon::UGunWeapon()
{
    WeaponId = TEXT("Gun");
    Cooldown = 0.5f;
}

bool UGunWeapon::TryAttack_Implementation()
{
    if (!IsCooldownReady()) return false;

    ACharacter* Owner = GetOwnerSafe();
    UCameraComponent* Cam = GetCameraSafe();
    if (!Owner || !Cam) return false;

    FireHitscan(FireDamage, EHitType::Normal);
    PlayMontage(FireMontage);
    StartCooldown();
    OnGunFired.Broadcast();
    return true;
}

void UGunWeapon::FireChargedShot(float Damage, float FireLockoutSeconds)
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld() || !GetCameraSafe()) return;

    FireHitscan(Damage, EHitType::Charged);
    ApplyCooldownAfterShot(FireLockoutSeconds);
    OnGunFired.Broadcast();
}

void UGunWeapon::FireRicochetShot(float Damage, int32 RicochetCount, float FireLockoutSeconds)
{
    ACharacter* Owner = GetOwnerSafe();
    UCameraComponent* Cam = GetCameraSafe();
    if (!Owner || !Cam || !Owner->GetWorld()) return;

    const FVector InitStart = Cam->GetComponentLocation();
    const FVector InitEnd   = InitStart + Cam->GetForwardVector() * FireRange;

    FHitResult InitHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    const bool bInitHit = Owner->GetWorld()->LineTraceSingleByChannel(
        InitHit, InitStart, InitEnd, ECC_Visibility, Params);

    SpawnTrailEffect(InitStart, bInitHit ? InitHit.ImpactPoint : InitEnd);

    if (!bInitHit || !InitHit.GetActor())
    {
        OnGunFired.Broadcast();
        ApplyCooldownAfterShot(FireLockoutSeconds);
        return;
    }

    // Apply damage to initial penetrated target
    const FWeaponHitResult InitResult = FHitResultBuilder()
        .From(Owner->GetController())
        .FromHit(InitHit)
        .WithDamage(Damage)
        .OfDamageType(EWeaponDamageType::Gun)
        .OfHitType(EHitType::Charged)
        .Build();

    OnGunHit.Broadcast(InitResult);
    FHitResultBuilder()
        .From(Owner->GetController())
        .FromHit(InitHit)
        .WithDamage(Damage)
        .OfDamageType(EWeaponDamageType::Gun)
        .OfHitType(EHitType::Charged)
        .Apply();

    // Ricochet chain
    FVector BounceOrigin    = InitHit.ImpactPoint;
    FVector BounceDirection = Cam->GetForwardVector();

    for (int32 i = 0; i < RicochetCount; ++i)
    {
        FVector NextOrigin;
        FVector NextDirection;

        if (!PerformRicochetBounce(BounceOrigin, BounceDirection, Damage, NextOrigin, NextDirection))
            break;

        BounceOrigin    = NextOrigin;
        BounceDirection = NextDirection;
    }

    OnGunFired.Broadcast();
    ApplyCooldownAfterShot(FireLockoutSeconds);
}

bool UGunWeapon::PerformRicochetBounce(const FVector& Origin, const FVector& InDirection,
                                       float Damage, FVector& OutBounceOrigin, FVector& OutBounceDirection)
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return false;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    AActor* NearestEnemy = FindNearestEnemy(Origin, Owner);

    FVector BounceDirection;

    if (NearestEnemy)
    {
        // Auto-target nearest enemy
        BounceDirection = (NearestEnemy->GetActorLocation() - Origin).GetSafeNormal();
    }
    else
    {
        // Real reflection off nearest wall
        const FVector RayEnd = Origin + InDirection * FireRange;

        FHitResult WallHit;
        if (!Owner->GetWorld()->LineTraceSingleByChannel(WallHit, Origin, RayEnd, ECC_WorldStatic, Params))
            return false;

        BounceDirection = FMath::GetReflectionVector(InDirection, WallHit.Normal);
    }

    const FVector BounceEnd = Origin + BounceDirection * FireRange;

    SpawnTrailEffect(Origin, BounceEnd);

    FHitResult BounceHit;
    const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
        BounceHit, Origin, BounceEnd, ECC_Visibility, Params);

    if (bHit && BounceHit.GetActor())
    {
        const FWeaponHitResult Result = FHitResultBuilder()
            .From(Owner->GetController())
            .FromHit(BounceHit)
            .WithDamage(Damage)
            .OfDamageType(EWeaponDamageType::Gun)
            .OfHitType(EHitType::Charged)
            .Build();

        OnGunHit.Broadcast(Result);
        FHitResultBuilder()
            .From(Owner->GetController())
            .FromHit(BounceHit)
            .WithDamage(Damage)
            .OfDamageType(EWeaponDamageType::Gun)
            .OfHitType(EHitType::Charged)
            .Apply();

        OutBounceOrigin    = BounceHit.ImpactPoint;
        OutBounceDirection = BounceDirection;
        return true;
    }

    OutBounceOrigin    = BounceEnd;
    OutBounceDirection = BounceDirection;
    return false;
}

AActor* UGunWeapon::FindNearestEnemy(const FVector& Origin, AActor* IgnoreActor) const
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return nullptr;

    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(IgnoreActor);

    Owner->GetWorld()->SweepMultiByChannel(
        Hits, Origin, Origin,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(AutoTargetRadius),
        Params);

    AActor* Nearest     = nullptr;
    float   NearestDist = FLT_MAX;

    for (const FHitResult& Hit : Hits)
    {
        AActor* Actor = Hit.GetActor();
        if (!Actor || Actor == IgnoreActor) continue;
        if (!Actor->Implements<UDamageable>()) continue;

        const float Dist = FVector::Dist(Origin, Actor->GetActorLocation());
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            Nearest     = Actor;
        }
    }

    return Nearest;
}

void UGunWeapon::SpawnTrailEffect(const FVector& Start, const FVector& End)
{
    if (!BulletTrailFX) return;

    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        Owner->GetWorld(), BulletTrailFX, Start,
        (End - Start).Rotation());
}

void UGunWeapon::FireHitscan(float Damage, EHitType HitType)
{
    ACharacter* Owner = GetOwnerSafe();
    UCameraComponent* Cam = GetCameraSafe();
    if (!Owner || !Cam || !Owner->GetWorld()) return;

    const FVector Start = Cam->GetComponentLocation();
    const FVector End   = Start + Cam->GetForwardVector() * FireRange;

    FHitResult EngineHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
        EngineHit, Start, End, ECC_Visibility, Params);

    if (!bHit || !EngineHit.GetActor()) return;

    const FWeaponHitResult Result = FHitResultBuilder()
        .From(Owner->GetController())
        .FromHit(EngineHit)
        .WithDamage(Damage)
        .OfDamageType(EWeaponDamageType::Gun)
        .OfHitType(HitType)
        .Build();

    OnGunHit.Broadcast(Result);
    FHitResultBuilder()
        .From(Owner->GetController())
        .FromHit(EngineHit)
        .WithDamage(Damage)
        .OfDamageType(EWeaponDamageType::Gun)
        .OfHitType(HitType)
        .Apply();
}

void UGunWeapon::ApplyCooldownAfterShot(float FireLockoutSeconds)
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    if (FireLockoutSeconds > 0.f)
        StartCooldownUntil(Owner->GetWorld()->GetTimeSeconds() + FireLockoutSeconds);
    else
        StartCooldown();
}