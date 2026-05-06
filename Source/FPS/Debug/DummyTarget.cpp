#include "Debug/DummyTarget.h"

ADummyTarget::ADummyTarget()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADummyTarget::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	OriginalScale = GetActorScale3D();
}

void ADummyTarget::OnWeaponHit_Implementation(const FWeaponHitResult& Hit)
{
	CurrentHealth = FMath::Max(0.f, CurrentHealth - Hit.Damage);

	ApplyHitFeedback();

	if (CurrentHealth <= 0.f)
		CurrentHealth = MaxHealth;
}

void ADummyTarget::ApplyHitFeedback()
{
	SetActorScale3D(OriginalScale * ShrinkScale);
	GetWorldTimerManager().ClearTimer(ScaleTimer);
	GetWorldTimerManager().SetTimer(ScaleTimer, this, &ADummyTarget::RestoreScale, ShrinkDuration, false);
}

void ADummyTarget::RestoreScale()
{
	SetActorScale3D(OriginalScale);
}