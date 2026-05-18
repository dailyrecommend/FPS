#include "Weapons/Implementations/Gun/GunWeapon.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

void UGunWeapon::FireHitscan(float Damage, EHitType HitType)
{
    ACharacter* Owner = GetOwnerSafe();
    UCameraComponent* Cam = GetCameraSafe();
    if (!Owner || !Cam || !Owner->GetWorld()) return;

    const FVector Start = Cam->GetComponentLocation();
    const FVector End   = Start + Cam->GetForwardVector() * FireRange;

    FVector TrailStart = Start;
    if (USceneComponent* Muzzle = Cast<USceneComponent>(
        Owner->GetDefaultSubobjectByName(MuzzlePointName)))
        TrailStart = Muzzle->GetComponentLocation();

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
        Hit, Start, End, ECC_Visibility, Params);

    if (!bHit)
    {
        SpawnTrailEffect(TrailStart, End);
        return;
    }

    SpawnTrailEffect(TrailStart, Hit.ImpactPoint);

    if (!Hit.GetActor() || !Hit.GetActor()->Implements<UDamageable>()) return;

    const FWeaponHitResult Result = FHitResultBuilder()
        .From(Owner->GetController())
        .FromHit(Hit)
        .WithDamage(Damage)
        .OfDamageType(EWeaponDamageType::Gun)
        .OfHitType(HitType)
        .Build();

    OnGunHit.Broadcast(Result);
    FHitResultBuilder()
        .From(Owner->GetController())
        .FromHit(Hit)
        .WithDamage(Damage)
        .OfDamageType(EWeaponDamageType::Gun)
        .OfHitType(HitType)
        .Apply();
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

    for (FTimerHandle& Handle : RicochetTimers)
        Owner->GetWorldTimerManager().ClearTimer(Handle);
    RicochetTimers.Empty();

    FVector TrailStart = Cam->GetComponentLocation();
    if (USceneComponent* Muzzle = Cast<USceneComponent>(
        Owner->GetDefaultSubobjectByName(MuzzlePointName)))
        TrailStart = Muzzle->GetComponentLocation();

    FVector RayStart = Cam->GetComponentLocation();
    FVector RayDir   = Cam->GetForwardVector();
    AActor* LastHit  = nullptr;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    // 적을 관통하며 직진, 벽을 만나면 도탄 시작
    while (true)
    {
        const FVector RayEnd = RayStart + RayDir * FireRange;

        FHitResult Hit;
        const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
            Hit, RayStart, RayEnd, ECC_Visibility, Params);

        if (!bHit)
        {
            SpawnTrailEffect(TrailStart, RayEnd);
            break;
        }

        AActor* HitActor    = Hit.GetActor();
        const bool bIsEnemy = HitActor && HitActor->Implements<UDamageable>();

        if (bIsEnemy)
        {
            // 적 — 데미지 + 관통 (도탄 횟수 차감 없음, 타이머 없음)
            SpawnTrailEffect(TrailStart, Hit.ImpactPoint);
            TrailStart = Hit.ImpactPoint;

            OnGunHit.Broadcast(FHitResultBuilder()
                .From(Owner->GetController())
                .FromHit(Hit)
                .WithDamage(Damage)
                .OfDamageType(EWeaponDamageType::Gun)
                .OfHitType(EHitType::Charged)
                .Build());

            FHitResultBuilder()
                .From(Owner->GetController())
                .FromHit(Hit)
                .WithDamage(Damage)
                .OfDamageType(EWeaponDamageType::Gun)
                .OfHitType(EHitType::Charged)
                .Apply();

            // 맞은 적 무시하고 같은 방향 직진
            LastHit  = HitActor;
            RayStart = Hit.ImpactPoint + RayDir * 5.f;
            Params.AddIgnoredActor(HitActor);
        }
        else
        {
            // 벽 — 트레일 그리고 도탄 체인 시작 (타이머)
            SpawnTrailEffect(TrailStart, Hit.ImpactPoint);

            if (RicochetCount > 0)
            {
                FVector NextDir;
                AActor* NearestEnemy = FindNearestEnemy(Hit.ImpactPoint, LastHit);
                if (NearestEnemy)
                    NextDir = (NearestEnemy->GetActorLocation() - Hit.ImpactPoint).GetSafeNormal();
                else
                    NextDir = FMath::GetReflectionVector(RayDir, Hit.Normal);

                const FVector NextOrigin = Hit.ImpactPoint + Hit.Normal * 2.f;
                ExecuteRicochetChain(0, RicochetCount, NextOrigin, NextDir, Damage, LastHit);
            }
            break;
        }
    }

    OnGunFired.Broadcast();
    ApplyCooldownAfterShot(FireLockoutSeconds);
}

void UGunWeapon::ExecuteRicochetChain(int32 BounceIndex, int32 TotalBounces,
                                      FVector Origin, FVector Direction,
                                      float Damage, AActor* LastHitActor)
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    if (BounceIndex >= TotalBounces) return;

    FTimerHandle Handle;
    Owner->GetWorldTimerManager().SetTimer(Handle, [=, this]()
    {
        ACharacter* O = GetOwnerSafe();
        if (!O || !O->GetWorld()) return;

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(O);

        FVector RayStart    = Origin;
        FVector RayDir      = Direction;
        FVector TrailStart  = Origin;
        AActor* CurrentLast = LastHitActor;

        if (CurrentLast) Params.AddIgnoredActor(CurrentLast);

        // 도탄 후에도 적을 관통하며 직진, 다음 벽에서 도탄
        while (true)
        {
            const FVector RayEnd = RayStart + RayDir * FireRange;

            FHitResult Hit;
            const bool bHit = O->GetWorld()->LineTraceSingleByChannel(
                Hit, RayStart, RayEnd, ECC_Visibility, Params);

            if (!bHit)
            {
                SpawnTrailEffect(TrailStart, RayEnd);
                break;
            }

            AActor* HitActor    = Hit.GetActor();
            const bool bIsEnemy = HitActor && HitActor->Implements<UDamageable>();

            if (bIsEnemy)
            {
                // 적 — 데미지 + 관통 (도탄 횟수 차감 없음)
                SpawnTrailEffect(TrailStart, Hit.ImpactPoint);
                TrailStart = Hit.ImpactPoint;

                OnGunHit.Broadcast(FHitResultBuilder()
                    .From(O->GetController())
                    .FromHit(Hit)
                    .WithDamage(Damage)
                    .OfDamageType(EWeaponDamageType::Gun)
                    .OfHitType(EHitType::Charged)
                    .Build());

                FHitResultBuilder()
                    .From(O->GetController())
                    .FromHit(Hit)
                    .WithDamage(Damage)
                    .OfDamageType(EWeaponDamageType::Gun)
                    .OfHitType(EHitType::Charged)
                    .Apply();

                CurrentLast = HitActor;
                RayStart    = Hit.ImpactPoint + RayDir * 5.f;
                Params.AddIgnoredActor(HitActor);
            }
            else
            {
                // 벽 — 도탄 횟수 1 차감 후 다음 도탄
                SpawnTrailEffect(TrailStart, Hit.ImpactPoint);

                FVector NextDir;
                AActor* NearestEnemy = FindNearestEnemy(Hit.ImpactPoint, CurrentLast);
                if (NearestEnemy)
                    NextDir = (NearestEnemy->GetActorLocation() - Hit.ImpactPoint).GetSafeNormal();
                else
                    NextDir = FMath::GetReflectionVector(RayDir, Hit.Normal);

                const FVector NextOrigin = Hit.ImpactPoint + Hit.Normal * 2.f;
                ExecuteRicochetChain(BounceIndex + 1, TotalBounces, NextOrigin, NextDir, Damage, CurrentLast);
                break;
            }
        }

    }, RicochetInterval, false);

    RicochetTimers.Add(Handle);
}

AActor* UGunWeapon::FindNearestEnemy(const FVector& Origin, AActor* IgnoreActor) const
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return nullptr;

    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);
    if (IgnoreActor) Params.AddIgnoredActor(IgnoreActor);

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
        if (!Actor) continue;
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

    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        Owner->GetWorld(),
        BulletTrailFX,
        Start,
        FRotator::ZeroRotator,
        FVector(1.f),
        true,
        true,
        ENCPoolMethod::None
    );

    if (!NiagaraComp) return;

    NiagaraComp->SetVectorParameter(TEXT("BeamStart"), Start);
    NiagaraComp->SetVectorParameter(TEXT("BeamEnd"),   End);
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

void UGunWeapon::EndPlay(EEndPlayReason::Type Reason)
{
    ACharacter* Owner = GetOwnerSafe();
    if (Owner && Owner->GetWorld())
    {
        for (FTimerHandle& Handle : RicochetTimers)
            Owner->GetWorldTimerManager().ClearTimer(Handle);
    }
    RicochetTimers.Empty();

    Super::EndPlay(Reason);
}