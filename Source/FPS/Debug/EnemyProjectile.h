#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

/**
 * 적이 발사하는 직선 발사체. 충돌한 IDamageable에 피해를 주고 소멸한다.
 *
 * 피해를 넘길 때 Attacker를 발사한 적으로 지정하므로, 플레이어가 패링하면
 * 방향 판정이 적 기준으로 이뤄지고 반격도 적에게 들어간다.
 */
UCLASS()
class FPS_API AEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	AEnemyProjectile();

	/**
	 * 지연 스폰 후 FinishSpawning 전에 호출해야 한다.
	 * BeginPlay에서 오버랩을 등록하므로, 그 전에 발사자가 정해져야 자기 자신에게 맞지 않는다.
	 */
	void InitProjectile(AActor* InShooter, float InDamage, float InSpeed);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
				   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, Category = "Projectile") TObjectPtr<USphereComponent>             Collision;
	UPROPERTY(VisibleAnywhere, Category = "Projectile") TObjectPtr<UStaticMeshComponent>         Mesh;
	UPROPERTY(VisibleAnywhere, Category = "Projectile") TObjectPtr<UProjectileMovementComponent> Movement;

	/** 충돌 크기는 Collision 컴포넌트의 Sphere Radius에서 직접 편집한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile") float LifeSeconds = 5.f;

	float Damage = 20.f;

	UPROPERTY()
	TWeakObjectPtr<AActor> Shooter;
};
