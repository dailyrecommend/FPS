#include "GunComponent.h"

#include "TimeScaleComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

UGunComponent::UGunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGunComponent::BeginPlay()
{
	Super::BeginPlay();
	if (OwnerCharacter)
	{
		CurrentFOV = OwnerCharacter->GetDefaultFOV();
		TargetFOV  = CurrentFOV;
	}
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

	// 집중 중이면 기본 공격 무시
	if (bIsFocusing) return;

	PerformHitscan(FireDamage);
	NotifyAnimationFired();
	OnGunFired.Broadcast();
}

void UGunComponent::PerformHitscan(float Damage)
{
	if (!Camera || !OwnerCharacter) return;

	FVector  Start = Camera->GetComponentLocation();
	FVector  End   = Start + Camera->GetForwardVector() * FireRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	if (OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params))
	{
		// 나중에 데미지 시스템 연결
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

void UGunComponent::StartFocus()
{
	if (!CanFocus()) return;
	bIsFocusing  = true;
	FocusElapsed = 0.f;

	// FOV는 바로 시작
	TargetFOV = OwnerCharacter->GetDefaultFOV() - FocusFOV;

	// 몽타주 바로 재생
	if (FocusMontage)
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetArmsMesh()->GetAnimInstance();
		if (AnimInstance)
			AnimInstance->Montage_Play(FocusMontage, 1.0f);
	}

	// 슬로우만 딜레이 후 적용
	OwnerCharacter->GetWorldTimerManager().SetTimer(FocusStartTimer, [this]()
	{
		if (!OwnerCharacter || !bIsFocusing) return;

		FTimeScaleParams Params;
		Params.Mode          = ETimeScaleMode::Full;
		Params.WorldDilation = FocusWorldDilation;
		Params.Duration      = 0.f;
		Params.BlendIn       = 0.1f;
		Params.BlendOut      = 0.2f;
		OwnerCharacter->GetTimeScaleComponent()->ApplyTimeScale(Params);
	}, FocusStartDelay, false);
}

void UGunComponent::EndFocus()
{
	
	OwnerCharacter->GetWorldTimerManager().ClearTimer(FocusStartTimer);
	
	if (!bIsFocusing) return;
	bIsFocusing            = false;
	FocusCooldownRemaining = FocusCooldown;
	LastFireTime           = OwnerCharacter->GetWorld()->GetTimeSeconds() + 0.5f;

	OwnerCharacter->GetTimeScaleComponent()->ClearTimeScale(0.f);
	TargetFOV = OwnerCharacter->GetDefaultFOV();

	// 집중 몽타주 중지 후 발사 몽타주 재생
	UAnimInstance* AnimInstance = OwnerCharacter->GetArmsMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.1f, FocusMontage);

		if (FocusFireMontage)
			AnimInstance->Montage_Play(FocusFireMontage, 1.0f);
	}

	PerformHitscan(FocusChargedDamage);
	OnGunFired.Broadcast();
}

void UGunComponent::TickComponent(float DeltaTime, ELevelTick TickType,
									FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float UnscaledDelta = FApp::GetDeltaTime();

	// FOV 보간
	if (Camera && OwnerCharacter)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, UnscaledDelta, FocusFOVInterpSpeed);
		Camera->SetFieldOfView(CurrentFOV);
	}

	// 최대 지속 시간 체크
	if (bIsFocusing)
	{
		FocusElapsed += UnscaledDelta;
		if (FocusElapsed >= FocusMaxDuration)
			EndFocus();
	}

	// 쿨타임 감소
	if (FocusCooldownRemaining > 0.f)
		FocusCooldownRemaining -= UnscaledDelta;
}

bool UGunComponent::CanFocus() const
{
	return !bIsFocusing && FocusCooldownRemaining <= 0.f;
}
