#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;

class UPlayerCharacterInputConfig;
class UPlayerInputRouter;

class UAbilityRegistry;
class UJumpAbility;
class UDashAbility;
class USlamAbility;
class UWallJumpAbility;
class UGlissandoAbility;

class UWeaponRegistry;
class UGunWeapon;
class USwordWeapon;
class UFocusSkill;
class UIajutsuSkill;

class UAnimationPlayerComponent;
class UCameraEffectsComponent;

UCLASS()
class FPS_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    UFUNCTION(BlueprintPure, Category = "Camera") UCameraComponent* GetCamera() const { return Camera; }

protected:
    virtual void PostInitializeComponents() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UPlayerCharacterInputConfig> InputConfig;

private:
    void RegisterInputMappingContext();
    void WirePresentation();
    void InjectAndRegisterAbilities();
    void InjectAndRegisterWeapons();

    //~ Visual root
    UPROPERTY(VisibleAnywhere, Category = "Camera") TObjectPtr<UCameraComponent>       Camera;
    UPROPERTY(VisibleAnywhere, Category = "Mesh")   TObjectPtr<USkeletalMeshComponent> ArmsMesh;

    //~ Input
    UPROPERTY(VisibleAnywhere, Category = "Input")  TObjectPtr<UPlayerInputRouter>     InputRouter;

    //~ Movement domain
    UPROPERTY(VisibleAnywhere, Category = "Movement") TObjectPtr<UAbilityRegistry>     AbilityRegistry;
    UPROPERTY(VisibleAnywhere, Category = "Movement") TObjectPtr<UJumpAbility>         JumpAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement") TObjectPtr<UDashAbility>         DashAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement") TObjectPtr<USlamAbility>         SlamAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement") TObjectPtr<UWallJumpAbility>     WallJumpAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement") TObjectPtr<UGlissandoAbility>    GlissandoAbility;

    //~ Weapons domain
    UPROPERTY(VisibleAnywhere, Category = "Weapon") TObjectPtr<UWeaponRegistry>        WeaponRegistry;
    UPROPERTY(VisibleAnywhere, Category = "Weapon") TObjectPtr<UGunWeapon>             GunWeapon;
    UPROPERTY(VisibleAnywhere, Category = "Weapon") TObjectPtr<USwordWeapon>           SwordWeapon;
    UPROPERTY(VisibleAnywhere, Category = "Weapon") TObjectPtr<UFocusSkill>            FocusSkill;
    UPROPERTY(VisibleAnywhere, Category = "Weapon") TObjectPtr<UIajutsuSkill>          IajutsuSkill;

    //~ Presentation
    UPROPERTY(VisibleAnywhere, Category = "Presentation") TObjectPtr<UAnimationPlayerComponent> AnimationPlayer;
    UPROPERTY(VisibleAnywhere, Category = "Presentation") TObjectPtr<UCameraEffectsComponent>   CameraEffects;
};