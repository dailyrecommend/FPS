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

void UWeaponBase::OnEquipped_Implementation()   { bIsEquipped = true; }
void UWeaponBase::OnUnequipped_Implementation() { bIsEquipped = false; }

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

void UWeaponBase::StartCooldownUntil(float WorldTimeSeconds)
{
	if (!OwnerCharacter.IsValid() || !OwnerCharacter->GetWorld()) return;
	LastUseTime = WorldTimeSeconds - Cooldown;
	if (LastUseTime < -BIG_NUMBER) LastUseTime = -BIG_NUMBER;
}

void UWeaponBase::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage) return;
	UObject* Player = AnimationPlayer.GetObject();
	if (!Player) return;
	IAnimationPlayer::Execute_PlayMontage(Player, Montage, PlayRate);
}

void UWeaponBase::StopMontage(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage) return;
	UObject* Player = AnimationPlayer.GetObject();
	if (!Player) return;
	IAnimationPlayer::Execute_StopMontage(Player, Montage, BlendOutTime);
}

ACharacter* UWeaponBase::GetOwnerSafe() const
{
	return OwnerCharacter.Get();
}

UCameraComponent* UWeaponBase::GetCameraSafe() const
{
	return Camera.Get();
}