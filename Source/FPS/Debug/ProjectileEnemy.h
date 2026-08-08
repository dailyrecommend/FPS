#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Combat/Interfaces/Damageable.h"
#include "ProjectileEnemy.generated.h"

class AEnemyProjectile;

/**
 * 테스트용 원거리 적. 사거리 안의 플레이어를 향해 일정 간격으로 발사체를 쏜다.
 * 피격 시 체력이 닳고, 0이 되면 ADummyTarget과 같은 방식으로 체력을 되돌린다.
 */
UCLASS()
class FPS_API AProjectileEnemy : public ACharacter, public IDamageable
{
	GENERATED_BODY()

public:
	AProjectileEnemy();

	virtual void  OnWeaponHit_Implementation(const FWeaponHitResult& Hit) override;
	virtual bool  IsAlive_Implementation() const override          { return CurrentHealth > 0.f; }
	virtual float GetCurrentHealth_Implementation() const override { return CurrentHealth; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	AActor* FindTarget() const;
	void    FaceTarget(const AActor* Target, float DeltaTime);
	void    Fire(const AActor* Target);
	void    ApplyHitFeedback();
	void    RestoreScale();

	UPROPERTY(EditDefaultsOnly, Category = "Enemy") float MaxHealth      = 300.f;
	UPROPERTY(EditDefaultsOnly, Category = "Enemy") float ShrinkScale    = 0.85f;
	UPROPERTY(EditDefaultsOnly, Category = "Enemy") float ShrinkDuration = 0.1f;

	/** 비워두면 아무것도 발사하지 않는다. BP에서 발사체 클래스를 지정할 것. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") TSubclassOf<AEnemyProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") float   FireInterval     = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") float   FireRange        = 5000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") float   ProjectileDamage = 20.f;
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") float   ProjectileSpeed  = 1200.f;
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") FVector MuzzleOffset     = FVector(60.f, 0.f, 40.f);

	/** 조준 지점을 대상 원점에서 위로 올린다. 캡슐 중심보다 가슴/머리를 노리게 한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") float AimHeightOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") bool  bFaceTarget    = true;
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Fire") float TurnSpeed      = 5.f;

	float CurrentHealth = 300.f;
	float FireTimer     = 0.f;

	FVector      OriginalScale = FVector::OneVector;
	FTimerHandle ScaleTimer;
};
