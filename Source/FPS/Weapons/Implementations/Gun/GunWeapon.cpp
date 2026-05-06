#include "Weapons/Implementations/Gun/GunWeapon.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

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
    if (!Owner || !Owner->GetWorld()) return;
    if (!GetCameraSafe()) return;

    FireHitscan(Damage, EHitType::Charged);

    if (FireLockoutSeconds > 0.f)
    {
        const float UnlockAt = Owner->GetWorld()->GetTimeSeconds() + FireLockoutSeconds;
        StartCooldownUntil(UnlockAt);
    }
    else
    {
        StartCooldown();
    }

    OnGunFired.Broadcast();
}

void UGunWeapon::FireHitscan(float Damage, EHitType HitType)
{
    ACharacter* Owner = GetOwnerSafe();
    UCameraComponent* Cam = GetCameraSafe();
    if (!Owner || !Cam || !Owner->GetWorld()) return;

    const FVector Start = Cam->GetComponentLocation();
    const FVector End   = Start + Cam->GetForwardVector() * FireRange;

    FHitResult            EngineHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
        EngineHit, Start, End, ECC_Visibility, Params);

    if (!bHit || !EngineHit.GetActor()) return;

    FHitResultBuilder Builder;
    Builder
        .From(Owner->GetController())
        .FromHit(EngineHit)
        .WithDamage(Damage)
        .OfDamageType(EWeaponDamageType::Gun)
        .OfHitType(HitType);

    OnGunHit.Broadcast(Builder.Build());
    Builder.Apply();
}