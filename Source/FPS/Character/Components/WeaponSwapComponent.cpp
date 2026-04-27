#include "WeaponSwapComponent.h"
#include "../PlayerCharacter.h"
#include "../Components/GunComponent.h"

UWeaponSwapComponent::UWeaponSwapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponSwapComponent::Initialize(APlayerCharacter* InOwner)
{
	OwnerCharacter = InOwner;
}

void UWeaponSwapComponent::SwapToGun()
{
	UE_LOG(LogTemp, Warning, TEXT("SwapToGun Called"));
	if (bIsSwapping || CurrentWeapon == EWeaponType::Gun) return;
	PerformSwap(EWeaponType::Gun);
}

void UWeaponSwapComponent::SwapToSword()
{
	UE_LOG(LogTemp, Warning, TEXT("SwapToSword Called"));
	if (bIsSwapping || CurrentWeapon == EWeaponType::Sword) return;
	PerformSwap(EWeaponType::Sword);
}

void UWeaponSwapComponent::SwapScroll(float ScrollValue)
{
	if (bIsSwapping) return;
	if (ScrollValue > 0.f) SwapToGun();
	else if (ScrollValue < 0.f) SwapToSword();
}

void UWeaponSwapComponent::PerformSwap(EWeaponType NewWeapon)
{
	bIsSwapping   = true;
	CurrentWeapon = NewWeapon;

	if (OwnerCharacter)
	{
		TArray<USkeletalMeshComponent*> Meshes;
		OwnerCharacter->GetComponents<USkeletalMeshComponent>(Meshes);
        
		for (USkeletalMeshComponent* Mesh : Meshes)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mesh Name: %s"), *Mesh->GetName());
			
			if (Mesh->GetName() == TEXT("WeaponGunMesh"))
				Mesh->SetVisibility(NewWeapon == EWeaponType::Gun);
			else if (Mesh->GetName() == TEXT("WeaponSwordMesh"))
				Mesh->SetVisibility(NewWeapon == EWeaponType::Sword);
		}
	}

	// Draw 애니메이션 재생
	UAnimMontage* DrawMontage = NewWeapon == EWeaponType::Gun ? GunDrawMontage : SwordDrawMontage;
	if (DrawMontage && OwnerCharacter->GetArmsMesh())
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetArmsMesh()->GetAnimInstance();
		if (AnimInstance)
			AnimInstance->Montage_Play(DrawMontage, 1.0f);
	}

	OwnerCharacter->GetWorldTimerManager().SetTimer(SwapTimer, [this]()
	{
		OnSwapComplete();
	}, SwapDuration, false);

	OnWeaponSwapped.Broadcast(CurrentWeapon);
}

void UWeaponSwapComponent::OnSwapComplete()
{
	bIsSwapping = false;
}