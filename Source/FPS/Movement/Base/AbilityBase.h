#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Movement/Interfaces/Ability.h"
#include "Presentation/Interfaces/AnimationPlayer.h"
#include "AbilityBase.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityStateChanged, bool, bIsActive);

UCLASS(Abstract, ClassGroup = Custom)
class FPS_API UAbilityBase
    : public UActorComponent
    , public IAbility
{
    GENERATED_BODY()

public:
    UAbilityBase();

    virtual void InjectDependencies(ACharacter* InOwner);

    void AttachAnimationPlayer(TScriptInterface<IAnimationPlayer> InPlayer) { AnimationPlayer = InPlayer; }

    UPROPERTY(BlueprintAssignable, Category = "Movement|Ability")
    FOnAbilityStateChanged OnAbilityStateChanged;

    virtual EActivationResult TryActivate_Implementation(const FAbilityContext& Context) override;
    virtual void  Deactivate_Implementation() override;
    virtual bool  IsAbilityActive_Implementation() const override         { return bIsActive; }
    virtual FName GetAbilityId_Implementation() const override            { return AbilityId; }
    virtual bool  CanBeInterruptedBy_Implementation(FName) const override { return false; }
    virtual bool  RequestCancel_Implementation() override;

protected:
    virtual EActivationResult OnTryActivate(const FAbilityContext& /*Context*/) { return EActivationResult::Success; }
    virtual void              OnDeactivate() {}
    virtual bool              CheckPreconditions(const FAbilityContext& /*Context*/) const { return true; }

    bool IsCooldownReady() const;
    void StartCooldown();

    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.f);
    void StopMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

    UCharacterMovementComponent* GetMoveComp() const;
    ACharacter*                  GetOwnerSafe() const;

    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    FName AbilityId = NAME_None;

    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    float Cooldown = 0.f;

    UPROPERTY()
    TWeakObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TScriptInterface<IAnimationPlayer> AnimationPlayer;

    bool bIsActive = false;

private:
    void  SetActiveInternal(bool bNewActive);
    float LastActivationTime = -BIG_NUMBER;
};