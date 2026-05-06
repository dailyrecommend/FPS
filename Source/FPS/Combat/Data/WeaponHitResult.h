#pragma once
#include "CoreMinimal.h"
#include "Combat/Enums/EHitType.h"
#include "Combat/Enums/EWeaponDamageType.h"
#include "WeaponHitResult.generated.h"

class AController;

USTRUCT(BlueprintType)
struct FPS_API FWeaponHitResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	FVector HitNormal = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	float Damage = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	EWeaponDamageType DamageType = EWeaponDamageType::None;

	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	EHitType HitType = EHitType::Normal;

	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	TObjectPtr<AController> Instigator = nullptr;
};