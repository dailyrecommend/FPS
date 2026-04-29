#include "GunComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Combat/DamageableInterface.h"

UGunComponent::UGunComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGunComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UGunComponent::Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera)
{
    OwnerCharacter = InOwner;
    Camera         = InCamera;
}

bool UGunComponent::CanFire() const
{
    if (!OwnerCharacter) return false;
    return (OwnerCharacter->GetWorld()->GetTimeSeconds() - LastFireTime) >= FireCooldown;
}

void UGunComponent::TryFire()
{
    if (!CanFire()) return;
    LastFireTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
    PerformHitscan(FireDamage, EHitType::Normal);
    PlayFireMontage();
    OnGunFired.Broadcast();
}

void UGunComponent::PerformChargedShot(float Damage, float FireLockout)
{
    LastFireTime = OwnerCharacter->GetWorld()->GetTimeSeconds() + FireLockout;
    PerformHitscan(Damage, EHitType::Charged);
    OnGunFired.Broadcast();
}

void UGunComponent::PerformHitscan(float Damage, EHitType HitType)
{
    if (!Camera || !OwnerCharacter) return;

    const FVector Start = Camera->GetComponentLocation();
    const FVector End   = Start + Camera->GetForwardVector() * FireRange;

    FHitResult            Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    if (!OwnerCharacter->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        return;

    if (!Hit.GetActor()) return;

    FWeaponHitResult WeaponHit;
    WeaponHit.HitActor    = Hit.GetActor();
    WeaponHit.HitLocation = Hit.ImpactPoint;
    WeaponHit.HitNormal   = Hit.ImpactNormal;
    WeaponHit.Damage      = Damage;
    WeaponHit.DamageType  = EWeaponDamageType::Gun;
    WeaponHit.HitType     = HitType;
    WeaponHit.bIsCritical = false;
    WeaponHit.Instigator  = OwnerCharacter->GetController();

    OnHit.Broadcast(WeaponHit);

    if (Hit.GetActor()->Implements<UDamageable>())
        IDamageable::Execute_OnWeaponHit(Hit.GetActor(), WeaponHit);
}

void UGunComponent::PlayFireMontage()
{
    if (!FireMontage || !OwnerCharacter) return;

    UAnimInstance* Anim = OwnerCharacter->GetArmsMesh()->GetAnimInstance();
    if (Anim) Anim->Montage_Play(FireMontage, 1.f);
}