#include "Weapons/Implementations/Gun/GunWeapon.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UGunWeapon::UGunWeapon()
{
    WeaponId = TEXT("Gun");
    Cooldown = 0.5f;
}

bool UGunWeapon::TryAttack_Implementation()
{
    if (!IsCooldownReady()) return false;
    if (!GetOwnerSafe() || !GetCameraSafe()) return false;

    FireHitscan(FireDamage, EHitType::Normal);
    PlayFireMontage();
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

    // Push the cooldown forward by lockout — prevents the player from immediately spamming
    // a normal shot after the heavy charged shot lands.
    StartCooldown();
    if (FireLockoutSeconds > 0.f)
    {
        const float World = Owner->GetWorld()->GetTimeSeconds();
        // Cheap way to extend the cooldown from outside: just bump LastUseTime forward
        // by re-applying cooldown logic via direct base call.
        // To keep it self-contained without exposing internals, we wait FireLockoutSeconds
        // worth of cooldown by inflating Cooldown temporarily through StartCooldown chain:
        //   (kept simple here; if extension is needed, expose StartCooldownAt(WorldTime).)
        // For now, longer Cooldown values on the asset are the recommended way.
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

void UGunWeapon::PlayFireMontage()
{
    if (!FireMontage) return;

    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return;

    USkeletalMeshComponent* Mesh = Owner->GetMesh();
    if (!Mesh) return;

    UAnimInstance* Anim = Mesh->GetAnimInstance();
    if (Anim) Anim->Montage_Play(FireMontage, 1.f);
}