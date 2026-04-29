#include "DummyTarget.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

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

	// 피격 색 전환을 위한 다이나믹 머티리얼 생성
	if (Mesh->GetMaterial(0))
	{
		DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
	}
}

float ADummyTarget::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
								AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnHit(DamageAmount);
	return ActualDamage;
}

void ADummyTarget::OnHit(float Damage)
{
	CurrentHealth -= Damage;

	// 빨간색으로 전환
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Red);

		GetWorldTimerManager().ClearTimer(ColorTimer);
		GetWorldTimerManager().SetTimer(ColorTimer, this, &ADummyTarget::RestoreColor,
										HitColorDuration, false);
	}

	// 크기 줄이기
	SetActorScale3D(OriginalScale * ShrinkScale);

	GetWorldTimerManager().ClearTimer(ScaleTimer);
	GetWorldTimerManager().SetTimer(ScaleTimer, this, &ADummyTarget::RestoreScale,
									ShrinkDuration, false);

	// 체력 0 이하면 리셋
	if (CurrentHealth <= 0.f)
		CurrentHealth = MaxHealth;
}

void ADummyTarget::RestoreColor()
{
	if (DynamicMaterial)
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
}

void ADummyTarget::RestoreScale()
{
	SetActorScale3D(OriginalScale);
}