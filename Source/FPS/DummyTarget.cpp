#include "DummyTarget.h"
#include "Combat/WeaponHitResult.h"

ADummyTarget::ADummyTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void ADummyTarget::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	OriginalScale = GetActorScale3D();
}

void ADummyTarget::OnWeaponHit_Implementation(const FWeaponHitResult& HitResult)
{
	OnHit(HitResult.Damage);
}

void ADummyTarget::OnHit(float Damage)
{
	CurrentHealth -= Damage;

	SetActorScale3D(OriginalScale * ShrinkScale);
	GetWorldTimerManager().ClearTimer(ScaleTimer);
	GetWorldTimerManager().SetTimer(ScaleTimer, this, &ADummyTarget::RestoreScale, ShrinkDuration, false);

	if (CurrentHealth <= 0.f)
		CurrentHealth = MaxHealth;
}

void ADummyTarget::RestoreScale()
{
	SetActorScale3D(OriginalScale);
}