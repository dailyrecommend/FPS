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

	UPROPERTY(BlueprintAssignable) FOnGunFired OnGunFired;

	UFUNCTION(BlueprintPure) bool CanFire() const;

protected:
	virtual void BeginPlay() override;

private:
	void PerformHitscan();
	void NotifyAnimationFired();

	UPROPERTY(EditDefaultsOnly, Category="Gun")
	float FireRange     = 10000.f;
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	float FireDamage    = 50.f;
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	float FireCooldown  = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	UAnimMontage* FireMontage;

	UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
	UPROPERTY() UCameraComponent* Camera         = nullptr;

	float LastFireTime = 0.f;
	
};