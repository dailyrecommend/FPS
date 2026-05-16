#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPSAnimInstance.generated.h"

UCLASS()
class FPS_API UFPSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
	float WeaponType = 0.f;

	void SetLocomotionState(float InSpeed, bool InAir, float InWeaponType)
	{
		Speed      = InSpeed;
		bIsInAir   = InAir;
		WeaponType = InWeaponType;
	}
};