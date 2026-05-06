#include "Combat/Builders/HitResultBuilder.h"
#include "Combat/Interfaces/Damageable.h"
#include "Engine/HitResult.h"
#include "GameFramework/Controller.h"

FHitResultBuilder& FHitResultBuilder::From(AController* InInstigator)
{
	Result.Instigator = InInstigator;
	return *this;
}

FHitResultBuilder& FHitResultBuilder::Target(AActor* Actor)
{
	Result.HitActor = Actor;
	return *this;
}

FHitResultBuilder& FHitResultBuilder::At(const FVector& Loc, const FVector& Normal)
{
	Result.HitLocation = Loc;
	Result.HitNormal   = Normal;
	return *this;
}

FHitResultBuilder& FHitResultBuilder::WithDamage(float Damage)
{
	Result.Damage = FMath::Max(0.f, Damage);
	return *this;
}

FHitResultBuilder& FHitResultBuilder::OfDamageType(EWeaponDamageType Type)
{
	Result.DamageType = Type;
	return *this;
}

FHitResultBuilder& FHitResultBuilder::OfHitType(EHitType Type)
{
	Result.HitType = Type;
	if (Type == EHitType::Critical || Type == EHitType::Headshot)
		Result.bIsCritical = true;
	return *this;
}

FHitResultBuilder& FHitResultBuilder::AsCritical(bool bCrit)
{
	Result.bIsCritical = bCrit;
	if (bCrit && Result.HitType == EHitType::Normal)
		Result.HitType = EHitType::Critical;
	return *this;
}

FHitResultBuilder& FHitResultBuilder::FromHit(const FHitResult& EngineHit)
{
	Result.HitActor    = EngineHit.GetActor();
	Result.HitLocation = EngineHit.ImpactPoint;
	Result.HitNormal   = EngineHit.ImpactNormal;
	return *this;
}

FWeaponHitResult FHitResultBuilder::Build() const
{
	return Result;
}

bool FHitResultBuilder::Apply() const
{
	if (!Result.HitActor) return false;
	if (!Result.HitActor->Implements<UDamageable>()) return false;

	IDamageable::Execute_OnWeaponHit(Result.HitActor, Result);
	return true;
}