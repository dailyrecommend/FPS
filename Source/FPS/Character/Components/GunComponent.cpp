#include "GunComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

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
	PerformHitscan();
	OnGunFired.Broadcast();
	NotifyAnimationFired();
}

void UGunComponent::PerformHitscan()
{
	if (!Camera || !OwnerCharacter) return;

	FVector  Start    = Camera->GetComponentLocation();
	FVector  End      = Start + Camera->GetForwardVector() * FireRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	if (OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params))
	{
		// 나중에 데미지 시스템 연결
		// Hit.GetActor() 으로 맞은 액터 가져올 수 있음
	}
}

void UGunComponent::NotifyAnimationFired()
{
	if (!OwnerCharacter || !FireMontage) return;

	USkeletalMeshComponent* ArmsMesh = OwnerCharacter->GetArmsMesh();
	if (!ArmsMesh) return;

	UAnimInstance* AnimInstance = ArmsMesh->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(FireMontage, 1.0f);
}