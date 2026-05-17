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

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    // 총구 트레일 시작점
    FVector TrailStart = Start;
    if (USceneComponent* Muzzle = Cast<USceneComponent>(
        Owner->GetDefaultSubobjectByName(MuzzlePointName)))
        TrailStart = Muzzle->GetComponentLocation();

    FHitResult Hit;
    const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
        Hit, Start, End, ECC_Visibility, Params);

    if (!bHit)
    {
        SpawnTrailEffect(TrailStart, End);
        return;
    }

    SpawnTrailEffect(TrailStart, Hit.ImpactPoint);

    // IDamageable인 경우에만 데미지
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

    // 총구 트레일 시작점
    FVector TrailStart = Cam->GetComponentLocation();
    if (USceneComponent* Muzzle = Cast<USceneComponent>(
        Owner->GetDefaultSubobjectByName(MuzzlePointName)))
        TrailStart = Muzzle->GetComponentLocation();

    FVector RayStart     = Cam->GetComponentLocation();
    FVector RayDir       = Cam->GetForwardVector();
    AActor* LastHitActor = nullptr;
    int32   BounceCount  = 0;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    while (true)
    {
        const FVector RayEnd = RayStart + RayDir * FireRange;

        FHitResult Hit;
        const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
            Hit, RayStart, RayEnd, ECC_Visibility, Params);

        if (!bHit)
        {
            // 아무것도 없음 — 트레일 그리고 종료
            SpawnTrailEffect(TrailStart, RayEnd);
            break;
        }

        AActor* HitActor  = Hit.GetActor();
        const bool bIsEnemy = HitActor && HitActor->Implements<UDamageable>();

        if (bIsEnemy)
        {
            // 적 맞음 — 데미지 + 관통 후 직진 계속
            SpawnTrailEffect(TrailStart, Hit.ImpactPoint);

            const FWeaponHitResult Result = FHitResultBuilder()
                .From(Owner->GetController())
                .FromHit(Hit)
                .WithDamage(Damage)
                .OfDamageType(EWeaponDamageType::Gun)
                .OfHitType(EHitType::Charged)
                .Build();

            OnGunHit.Broadcast(Result);
            FHitResultBuilder()
                .From(Owner->GetController())
                .FromHit(Hit)
                .WithDamage(Damage)
                .OfDamageType(EWeaponDamageType::Gun)
                .OfHitType(EHitType::Charged)
                .Apply();

            // 관통 — 맞은 적 무시하고 계속 직진
            TrailStart   = Hit.ImpactPoint;
            RayStart     = Hit.ImpactPoint + RayDir * 5.f;
            LastHitActor = HitActor;
            Params.AddIgnoredActor(HitActor);
        }
        else
        {
            // 벽/사물 맞음 — 도탄 처리
            SpawnTrailEffect(TrailStart, Hit.ImpactPoint);

            if (BounceCount >= RicochetCount) break;

            BounceCount++;

            // 근처 적 자동 타겟 확인
            AActor* NearestEnemy = FindNearestEnemy(Hit.ImpactPoint, LastHitActor);

            if (NearestEnemy)
            {
                RayDir = (NearestEnemy->GetActorLocation() - Hit.ImpactPoint).GetSafeNormal();
            }
            else
            {
                RayDir = FMath::GetReflectionVector(RayDir, Hit.Normal);
            }

            TrailStart = Hit.ImpactPoint;
            RayStart   = Hit.ImpactPoint + Hit.Normal * 2.f;
        }
    }

    OnGunFired.Broadcast();
    ApplyCooldownAfterShot(FireLockoutSeconds);
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