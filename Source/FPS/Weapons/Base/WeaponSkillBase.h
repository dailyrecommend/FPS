#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/Interfaces/WeaponSkill.h"
#include "Presentation/Interfaces/AnimationPlayer.h"
#include "Presentation/Interfaces/CameraEffects.h"
#include "WeaponSkillBase.generated.h"

class ACharacter;
class UCameraComponent;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillStateChanged, bool, bIsActive);

UCLASS(Abstract, ClassGroup = Custom)
class FPS_API UWeaponSkillBase
    : public UActorComponent
    , public IWeaponSkill
{
    GENERATED_BODY()

public:
    UWeaponSkillBase();

    virtual void InjectDependencies(ACharacter* InOwner, UCameraComponent* InCamera);

    void AttachAnimationPlayer(TScriptInterface<IAnimationPlayer> InPlayer) { AnimationPlayer = InPlayer; }
    void AttachCameraEffects(TScriptInterface<ICameraEffects> InEffects)    { CameraEffects   = InEffects; }

    UPROPERTY(BlueprintAssignable, Category = "Skill")
    FOnSkillStateChanged OnSkillStateChanged;

    UFUNCTION(BlueprintPure, Category = "Skill") float GetCooldownRemaining() const { return CooldownRemaining; }
    UFUNCTION(BlueprintPure, Category = "Skill") bool  CanActivate()          const;

    virtual void StartHold_Implementation()       override;
    virtual void EndHold_Implementation()         override;
    virtual bool IsSkillActive_Implementation()  const override { return bIsActive; }
    virtual bool BlocksMovement_Implementation() const override { return bIsActive; }

protected:
    virtual bool OnStartHold() { return true; }
    virtual void OnEndHold()   {}
    virtual void OnCancel()    {}

    void TickCooldown(float UnscaledDelta);

    void StartCooldown(float Duration);
    void Cancel();

    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.f);
    void StopMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

    int32 PushFOVOffset(float Offset, float InterpSpeed, int32 Priority);
    void  PopFOVOffset(int32 Handle);

    ACharacter*       GetOwnerSafe()  const;
    UCameraComponent* GetCameraSafe() const;

    UPROPERTY()
    TWeakObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TWeakObjectPtr<UCameraComponent> Camera;

    UPROPERTY()
    TScriptInterface<IAnimationPlayer> AnimationPlayer;

    UPROPERTY()
    TScriptInterface<ICameraEffects> CameraEffects;

    bool  bIsActive         = false;
    float CooldownRemaining = 0.f;

private:
    void SetActiveInternal(bool bNewActive);
};