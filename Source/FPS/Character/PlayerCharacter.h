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
class USlideAbility;

class UWeaponRegistry;
class UGunWeapon;
class USwordWeapon;
class UGunSkill;
class USwordSkill;

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

    UPROPERTY(VisibleAnywhere, Category = "Camera")       TObjectPtr<UCameraComponent>             Camera;
    UPROPERTY(VisibleAnywhere, Category = "Mesh")         TObjectPtr<USkeletalMeshComponent>       ArmsMesh;
    UPROPERTY(VisibleAnywhere, Category = "Input")        TObjectPtr<UPlayerInputRouter>           InputRouter;

    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<UAbilityRegistry>             AbilityRegistry;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<UJumpAbility>                 JumpAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<UDashAbility>                 DashAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<USlamAbility>                 SlamAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<UWallJumpAbility>             WallJumpAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<USlideAbility>                SlideAbility;

    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<UWeaponRegistry>              WeaponRegistry;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<UGunWeapon>                   GunWeapon;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<USwordWeapon>                 SwordWeapon;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<UGunSkill>                    GunSkill;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<USwordSkill>                  SwordSkill;

    UPROPERTY(VisibleAnywhere, Category = "Presentation") TObjectPtr<UAnimationPlayerComponent>    AnimationPlayer;
    UPROPERTY(VisibleAnywhere, Category = "Presentation") TObjectPtr<UCameraEffectsComponent>      CameraEffects;
};