#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponSwapComponent.generated.h"

class APlayerCharacter;
class UGunComponent;
class USwordComponent;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Gun,
	Sword,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwapped, EWeaponType, NewWeapon);

UCLASS(ClassGroup=Custom, meta=(BlueprintSpawnableComponent))
class FPS_API UWeaponSwapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponSwapComponent();

	void Initialize(APlayerCharacter* InOwner);

	void SwapToGun();
	void SwapToSword();
	void SwapScroll(float ScrollValue);

	UPROPERTY(BlueprintAssignable) FOnWeaponSwapped OnWeaponSwapped;

	UFUNCTION(BlueprintPure) EWeaponType GetCurrentWeapon() const { return CurrentWeapon; }
	UFUNCTION(BlueprintPure) bool IsSwapping() const { return bIsSwapping; }

protected:


private:
	void PerformSwap(EWeaponType NewWeapon);
	void OnSwapComplete();

	UPROPERTY(EditDefaultsOnly, Category="WeaponSwap")
	float SwapDuration = 0.5f;  // Draw 애니메이션 시간

	UPROPERTY(EditDefaultsOnly, Category="WeaponSwap")
	UAnimMontage* GunDrawMontage;

	UPROPERTY(EditDefaultsOnly, Category="WeaponSwap")
	UAnimMontage* SwordDrawMontage;

	UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;

	EWeaponType CurrentWeapon = EWeaponType::Gun;
	bool        bIsSwapping   = false;
	float       SwapElapsed   = 0.f;
	FTimerHandle SwapTimer;
};