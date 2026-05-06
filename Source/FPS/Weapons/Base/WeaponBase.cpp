#include "Weapons/Base/WeaponBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

UWeaponBase::UWeaponBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponBase::InjectDependencies(ACharacter* InOwner, UCameraComponent* InCamera)
{
	OwnerCharacter = InOwner;
	Camera         = InCamera;
}

void UWeaponBase::OnEquipped_Implementation()
{
	bIsEquipped = true;
}

void UWeaponBase::OnUnequipped_Implementation()
{
	bIsEquipped = false;
}

bool UWeaponBase::IsCooldownReady() const
{
	if (Cooldown <= 0.f) return true;
	if (!OwnerCharacter.IsValid() || !OwnerCharacter->GetWorld()) return false;
	return (OwnerCharacter->GetWorld()->GetTimeSeconds() - LastUseTime) >= Cooldown;
}

void UWeaponBase::StartCooldown()
{
	if (OwnerCharacter.IsValid() && OwnerCharacter->GetWorld())
		LastUseTime = OwnerCharacter->GetWorld()->GetTimeSeconds();
}

ACharacter* UWeaponBase::GetOwnerSafe() const
{
	return OwnerCharacter.Get();
}

UCameraComponent* UWeaponBase::GetCameraSafe() const
{
	return Camera.Get();
}