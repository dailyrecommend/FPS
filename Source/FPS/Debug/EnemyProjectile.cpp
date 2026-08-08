#include "Debug/EnemyProjectile.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Combat/Interfaces/Damageable.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

AEnemyProjectile::AEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(16.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->SetGenerateOverlapEvents(true);
	SetRootComponent(Collision);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->bRotationFollowsVelocity = true;
	Movement->ProjectileGravityScale   = 0.f;
	Movement->InitialSpeed             = 1200.f;
	Movement->MaxSpeed                 = 1200.f;
}

void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSeconds);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyProjectile::OnOverlap);
}

void AEnemyProjectile::InitProjectile(AActor* InShooter, float InDamage, float InSpeed)
{
	Shooter = InShooter;
	Damage  = InDamage;

	if (InShooter)
		Collision->IgnoreActorWhenMoving(InShooter, true);

	// FinishSpawning 시점의 InitializeComponent가 액터 정면 방향으로 속도를 잡아준다.
	// 여기서 Velocity를 직접 넣으면 그 초기화에 덮이므로 속도만 지정한다.
	Movement->InitialSpeed = InSpeed;
	Movement->MaxSpeed     = InSpeed;
}

void AEnemyProjectile::OnOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
								 UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
								 bool /*bFromSweep*/, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	if (Shooter.IsValid() && OtherActor == Shooter.Get()) return;
	if (OtherActor == GetOwner()) return;   // InitProjectile을 거치지 않은 경우의 안전망

	if (OtherActor->Implements<UDamageable>())
	{
		AActor* AttackerActor = Shooter.IsValid() ? Shooter.Get() : this;

		AController* InstigatorController = nullptr;
		if (APawn* ShooterPawn = Cast<APawn>(AttackerActor))
			InstigatorController = ShooterPawn->GetController();

		FHitResultBuilder()
			.From(InstigatorController)
			.By(AttackerActor)
			.Target(OtherActor)
			.At(GetActorLocation(), -GetActorForwardVector())
			.WithDamage(Damage)
			.OfDamageType(EWeaponDamageType::None)
			.Apply();
	}

	Destroy();
}
