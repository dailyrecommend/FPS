#include "Debug/ProjectileEnemy.h"
#include "Debug/EnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AProjectileEnemy::AProjectileEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectileEnemy::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	OriginalScale = GetActorScale3D();
	FireTimer     = 0.f;
}

void AProjectileEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AActor* Target = FindTarget();
	if (!Target)
	{
		FireTimer = 0.f;
		return;
	}

	if (bFaceTarget)
		FaceTarget(Target, DeltaTime);

	FireTimer += DeltaTime;
	if (FireTimer >= FireInterval)
	{
		FireTimer = 0.f;
		Fire(Target);
	}
}

AActor* AProjectileEnemy::FindTarget() const
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player) return nullptr;

	// 죽은 대상은 노리지 않는다.
	if (Player->Implements<UDamageable>() && !IDamageable::Execute_IsAlive(Player))
		return nullptr;

	const float DistSq = FVector::DistSquared(Player->GetActorLocation(), GetActorLocation());
	if (DistSq > FireRange * FireRange) return nullptr;

	return Player;
}

void AProjectileEnemy::FaceTarget(const AActor* Target, float DeltaTime)
{
	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;
	if (ToTarget.IsNearlyZero()) return;

	const FRotator Desired = ToTarget.Rotation();
	const FRotator Smooth  = FMath::RInterpTo(GetActorRotation(), Desired, DeltaTime, TurnSpeed);
	SetActorRotation(FRotator(0.f, Smooth.Yaw, 0.f));
}

void AProjectileEnemy::Fire(const AActor* Target)
{
	if (!ProjectileClass || !GetWorld()) return;

	const FVector MuzzleLocation = GetActorLocation() + GetActorRotation().RotateVector(MuzzleOffset);
	const FVector AimPoint       = Target->GetActorLocation() + FVector(0.f, 0.f, AimHeightOffset);

	FVector AimDirection = (AimPoint - MuzzleLocation).GetSafeNormal();
	if (AimDirection.IsNearlyZero()) return;

	const FTransform SpawnTransform(AimDirection.Rotation(), MuzzleLocation);

	// 지연 스폰이라야 BeginPlay(오버랩 등록) 전에 발사자를 지정할 수 있다.
	// 즉시 스폰하면 큰 발사체가 스폰 순간 발사자와 겹쳐 자기 자신을 때린다.
	AEnemyProjectile* Projectile = GetWorld()->SpawnActorDeferred<AEnemyProjectile>(
		ProjectileClass, SpawnTransform, this, this,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Projectile) return;

	Projectile->InitProjectile(this, ProjectileDamage, ProjectileSpeed);
	UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);
}

void AProjectileEnemy::OnWeaponHit_Implementation(const FWeaponHitResult& Hit)
{
	CurrentHealth = FMath::Max(0.f, CurrentHealth - Hit.Damage);

	ApplyHitFeedback();

	if (CurrentHealth <= 0.f)
		CurrentHealth = MaxHealth;
}

void AProjectileEnemy::ApplyHitFeedback()
{
	SetActorScale3D(OriginalScale * ShrinkScale);
	GetWorldTimerManager().ClearTimer(ScaleTimer);
	GetWorldTimerManager().SetTimer(ScaleTimer, this, &AProjectileEnemy::RestoreScale, ShrinkDuration, false);
}

void AProjectileEnemy::RestoreScale()
{
	SetActorScale3D(OriginalScale);
}
