#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGunFired);

UCLASS(ClassGroup=Custom, meta=(BlueprintSpawnableComponent))
class FPS_API UGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunComponent();

	void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera);
	void TryFire();
	void StartFocus();
	void EndFocus();

	UPROPERTY(BlueprintAssignable) FOnGunFired OnGunFired;

	UFUNCTION(BlueprintPure) bool CanFire() const;
	UFUNCTION(BlueprintPure) bool IsFocusing() const { return bIsFocusing; }
	UFUNCTION(BlueprintPure) bool  CanFocus() const;
	UFUNCTION(BlueprintPure) float GetFocusCooldownRemaining() const { return FocusCooldownRemaining; }


	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusSensitivity    = 0.4f;    // 감도 배율
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	void PerformHitscan(float Damage);
	void NotifyAnimationFired();
	


	UPROPERTY(EditDefaultsOnly, Category="Gun")
	float FireRange     = 10000.f;
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	float FireDamage    = 50.f;
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	float FireCooldown  = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusWorldDilation  = 0.15f;
	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusFOV            = 60.f;    // 기본 FOV에서 줄어들 값
	
	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusChargedDamage  = 500.f;   // 집중 중 발사 시 데미지
	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusFOVInterpSpeed = 8.f;
	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusMaxDuration = 3.f;     // 최대 지속 시간
	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusCooldown    = 5.f;     // 쿨타임

	UPROPERTY(EditDefaultsOnly, Category="Focus")
	UAnimMontage* FocusMontage;

	UPROPERTY(EditDefaultsOnly, Category="Focus")
	UAnimMontage* FocusFireMontage;
	
	UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
	UPROPERTY() UCameraComponent* Camera         = nullptr;

	float LastFireTime = 0.f;
	bool bIsFocusing = false;

	float CurrentFOV    = 90.f;
	float TargetFOV     = 90.f;
	float FocusElapsed   = 0.f;
	float FocusCooldownRemaining = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusStartDelay = 0.05f;

	FTimerHandle FocusStartTimer;
};