#include "GunComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

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
    PerformHitscan(FireDamage);
    PlayFireMontage();
    OnGunFired.Broadcast();
}

void UGunComponent::PerformChargedShot(float Damage, float FireLockout)
{
    // Push LastFireTime into the future to enforce the lockout window
    LastFireTime = OwnerCharacter->GetWorld()->GetTimeSeconds() + FireLockout;
    PerformHitscan(Damage);
    OnGunFired.Broadcast();
}

void UGunComponent::PerformHitscan(float Damage)
{
    if (!Camera || !OwnerCharacter) return;

    const FVector Start = Camera->GetComponentLocation();
    const FVector End   = Start + Camera->GetForwardVector() * FireRange;

    FHitResult            Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    if (OwnerCharacter->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        if (Hit.GetActor())
        {
            UGameplayStatics::ApplyDamage(
                Hit.GetActor(),
                FireDamage,
                OwnerCharacter->GetController(),
                OwnerCharacter,
                UDamageType::StaticClass()
            );
        }
    }
}

void UGunComponent::PlayFireMontage()
{
    if (!FireMontage || !OwnerCharacter) return;

    USkeletalMeshComponent* Arms = OwnerCharacter->GetArmsMesh();
    if (!Arms) return;

    UAnimInstance* Anim = Arms->GetAnimInstance();
    if (Anim) Anim->Montage_Play(FireMontage, 1.f);
}