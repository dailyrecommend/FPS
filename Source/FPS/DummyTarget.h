#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DummyTarget.generated.h"

/**
 * 피격 테스트용 더미 액터.
 * 맞으면 잠깐 빨간색으로 변했다가 원래 색으로 복귀.
 * 체력이 0이 되면 원래 크기로 돌아온 뒤 체력 리셋 (파괴 없음).
 */
UCLASS()
class FPS_API ADummyTarget : public AActor
{
	GENERATED_BODY()

public:
	ADummyTarget();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
							 AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

private:
	void OnHit(float Damage);
	void RestoreColor();
	void RestoreScale();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Dummy")
	float MaxHealth = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dummy")
	float HitColorDuration = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Dummy")
	float ShrinkScale = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Dummy")
	float ShrinkDuration = 0.1f;

	float CurrentHealth = 300.f;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;

	FTimerHandle ColorTimer;
	FTimerHandle ScaleTimer;

	FVector OriginalScale = FVector::OneVector;
};