#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/Interfaces/Weapon.h"
#include "Weapons/Interfaces/WeaponSkill.h"
#include "Presentation/Interfaces/AnimationPlayer.h"
#include "WeaponBase.generated.h"

class ACharacter;
class UCameraComponent;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponAttack);

UCLASS(Abstract, ClassGroup = Custom)
class FPS_API UWeaponBase
    : public UActorComponent
    , public IWeapon
{
    GENERATED_BODY()

public:
    UWeaponBase();

    virtual void InjectDependencies(ACharacter* InOwner, UCameraComponent* InCamera);

    void AttachSkill(TScriptInterface<IWeaponSkill> InSkill)             { Skill           = InSkill; }
    void AttachAnimationPlayer(TScriptInterface<IAnimationPlayer> InPlayer) { AnimationPlayer = InPlayer; }

    UPROPERTY(BlueprintAssignable, Category = "Weapon")
    FOnWeaponAttack OnWeaponAttack;

    virtual bool                          TryAttack_Implementation() override { return false; }
    virtual TScriptInterface<IWeaponSkill> GetSkill_Implementation() const override { return Skill; }
    virtual void                          OnEquipped_Implementation() override;
    virtual void                          OnUnequipped_Implementation() override;
    virtual FName                         GetWeaponId_Implementation() const override { return WeaponId; }
    virtual bool                          BlocksMovement_Implementation() const override { return false; }

protected:
    bool IsCooldownReady() const;
    void StartCooldown();
    void StartCooldownUntil(float WorldTimeSeconds);

    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.f);
    void StopMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

    ACharacter*       GetOwnerSafe()  const;
    UCameraComponent* GetCameraSafe() const;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponId = NAME_None;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float Cooldown = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Anim")
    TObjectPtr<UAnimMontage> DrawMontage;

    UPROPERTY()
    TWeakObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TWeakObjectPtr<UCameraComponent> Camera;

    UPROPERTY()
    TScriptInterface<IWeaponSkill> Skill;

    UPROPERTY()
    TScriptInterface<IAnimationPlayer> AnimationPlayer;

    bool bIsEquipped = false;

private:
    float LastUseTime = -BIG_NUMBER;
};