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

	/** 피격 대상. */
	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	TObjectPtr<AActor> HitActor = nullptr;

	/**
	 * 공격을 가한 액터. 방향 판정(패링 등)과 반격 대상으로 쓴다.
	 * Instigator와 달리 컨트롤러가 없는 발사체·함정도 담을 수 있다.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Hit")
	TObjectPtr<AActor> Attacker = nullptr;

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