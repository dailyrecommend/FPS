#pragma once
#include "CoreMinimal.h"
#include "WeaponHitResult.generated.h"

UENUM(BlueprintType)
enum class EWeaponDamageType : uint8
{
	Gun,
	Sword,
};

UENUM(BlueprintType)
enum class EHitType : uint8
{
	Normal,
	Charged,    // Focus skill
	Iajutsu,
};

USTRUCT(BlueprintType)
struct FPS_API FWeaponHitResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) AActor*           HitActor    = nullptr;
	UPROPERTY(BlueprintReadOnly) FVector           HitLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FVector           HitNormal   = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) float             Damage      = 0.f;
	UPROPERTY(BlueprintReadOnly) EWeaponDamageType DamageType  = EWeaponDamageType::Gun;
	UPROPERTY(BlueprintReadOnly) EHitType          HitType     = EHitType::Normal;
	UPROPERTY(BlueprintReadOnly) bool              bIsCritical = false;
	UPROPERTY(BlueprintReadOnly) AController*      Instigator  = nullptr;
};