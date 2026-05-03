#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Combat/DamageableInterface.h"
#include "DummyTarget.generated.h"

UCLASS()
class FPS_API ADummyTarget : public ACharacter, public IDamageable
{
	GENERATED_BODY()

public:
	ADummyTarget();

	virtual void OnWeaponHit_Implementation(const FWeaponHitResult& HitResult) override;

protected:
	virtual void BeginPlay() override;

private:
	void OnHit(float Damage);
	void RestoreScale();

	UPROPERTY(EditDefaultsOnly, Category = "Dummy") float MaxHealth     = 300.f;
	UPROPERTY(EditDefaultsOnly, Category = "Dummy") float ShrinkScale   = 0.85f;
	UPROPERTY(EditDefaultsOnly, Category = "Dummy") float ShrinkDuration = 0.1f;

	float   CurrentHealth = 300.f;
	FVector OriginalScale = FVector::OneVector;

	FTimerHandle ScaleTimer;
};