#include "WeaponSwapComponent.h"
#include "../PlayerCharacter.h"

UWeaponSwapComponent::UWeaponSwapComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponSwapComponent::Initialize(APlayerCharacter* InOwner,
                                       USkeletalMeshComponent* InGunMesh,
                                       USkeletalMeshComponent* InSwordMesh)
{
    OwnerCharacter = InOwner;
    GunMesh        = InGunMesh;
    SwordMesh      = InSwordMesh;
}

void UWeaponSwapComponent::SwapToGun()
{
    if (bIsSwapping || CurrentWeapon == EWeaponType::Gun) return;
    PerformSwap(EWeaponType::Gun);
}

void UWeaponSwapComponent::SwapToSword()
{
    if (bIsSwapping || CurrentWeapon == EWeaponType::Sword) return;
    PerformSwap(EWeaponType::Sword);
}

void UWeaponSwapComponent::SwapScroll(float ScrollValue)
{
    if (bIsSwapping) return;
    if (ScrollValue > 0.f)       SwapToGun();
    else if (ScrollValue < 0.f)  SwapToSword();
}

void UWeaponSwapComponent::PerformSwap(EWeaponType NewWeapon)
{
    bIsSwapping   = true;
    CurrentWeapon = NewWeapon;

    UpdateMeshVisibility(NewWeapon);
    PlayDrawMontage(NewWeapon);

    OwnerCharacter->GetWorldTimerManager().SetTimer(SwapTimer, [this]()
    {
        OnSwapComplete();
    }, SwapDuration, false);

    OnWeaponSwapped.Broadcast(CurrentWeapon);
}

void UWeaponSwapComponent::UpdateMeshVisibility(EWeaponType NewWeapon) const
{
    if (GunMesh)   GunMesh->SetVisibility(NewWeapon == EWeaponType::Gun);
    if (SwordMesh) SwordMesh->SetVisibility(NewWeapon == EWeaponType::Sword);
}

void UWeaponSwapComponent::PlayDrawMontage(EWeaponType NewWeapon) const
{
    UAnimMontage* Montage = (NewWeapon == EWeaponType::Gun) ? GunDrawMontage : SwordDrawMontage;
    if (!Montage || !OwnerCharacter) return;

    USkeletalMeshComponent* Arms = OwnerCharacter->GetArmsMesh();
    if (!Arms) return;

    UAnimInstance* Anim = Arms->GetAnimInstance();
    if (Anim) Anim->Montage_Play(Montage, 1.f);
}

void UWeaponSwapComponent::OnSwapComplete()
{
    bIsSwapping = false;
}