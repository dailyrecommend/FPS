#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/Interfaces/Weapon.h"
#include "WeaponBase.generated.h"

class ACharacter;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponAttack);

UCLASS(Abstract, ClassGroup = Custom)
class FPS_API UWeaponBase
	: public UActorComponent
	, public IWeapon
{
	GENERATED_BODY()

public:
	UWeaponBase();

	/** External dependency injection — owner and camera are passed in, never looked up. */
	virtual void InjectDependencies(ACharacter* InOwner, UCameraComponent* InCamera);

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponAttack OnWeaponAttack;

	virtual bool                          TryAttack_Implementation() override { return false; }
	virtual TScriptInterface<IWeaponSkill> GetSkill_Implementation() const override { return nullptr; }
	virtual void                          OnEquipped_Implementation() override;
	virtual void                          OnUnequipped_Implementation() override;
	virtual FName                         GetWeaponId_Implementation() const override { return WeaponId; }
	virtual bool                          BlocksMovement_Implementation() const override { return false; }

protected:
	bool IsCooldownReady() const;
	void StartCooldown();

	ACharacter*       GetOwnerSafe()  const;
	UCameraComponent* GetCameraSafe() const;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponId = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Cooldown = 0.5f;

	UPROPERTY()
	TWeakObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TWeakObjectPtr<UCameraComponent> Camera;

	bool bIsEquipped = false;

private:
	float LastUseTime = -BIG_NUMBER;
};