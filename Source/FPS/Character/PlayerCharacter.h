#pragma once
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapons/Data/WeaponChangedEvent.h"
#include "Combat/Interfaces/Damageable.h"
#include "UI/PlayerHUDWidget.h"
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
class UHealthComponent;
class UPlayerHUDWidget;

UCLASS()
class FPS_API APlayerCharacter : public ACharacter, public IDamageable
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    UFUNCTION(BlueprintPure, Category = "Camera") UCameraComponent*  GetCamera()        const { return Camera; }
    UFUNCTION(BlueprintPure, Category = "Health") UHealthComponent*  GetHealth()        const { return Health; }
    UFUNCTION(BlueprintPure, Category = "Weapon") UWeaponRegistry*   GetWeaponRegistry() const { return WeaponRegistry; }
    UFUNCTION(BlueprintPure, Category = "Movement") UDashAbility*    GetDashAbility()   const { return DashAbility; }

    virtual void  OnWeaponHit_Implementation(const FWeaponHitResult& Hit) override;
    virtual bool  IsAlive_Implementation()         const override;
    virtual float GetCurrentHealth_Implementation() const override;

protected:
    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UPlayerCharacterInputConfig> InputConfig;


private:
    void RegisterInputMappingContext();
    void WirePresentation();
    void InjectAndRegisterAbilities();
    void InjectAndRegisterWeapons();
    void TickLocomotion(float DeltaTime);

    UFUNCTION()
    void OnWeaponChanged(const FWeaponChangedEvent& Event);

    UPROPERTY(VisibleAnywhere, Category = "Camera")       TObjectPtr<UCameraComponent>          Camera;
    UPROPERTY(VisibleAnywhere, Category = "Mesh")         TObjectPtr<USkeletalMeshComponent>    ArmsMesh;
    UPROPERTY(VisibleAnywhere, Category = "Input")        TObjectPtr<UPlayerInputRouter>        InputRouter;

    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<UAbilityRegistry>          AbilityRegistry;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<UJumpAbility>              JumpAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<UDashAbility>              DashAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<USlamAbility>              SlamAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<UWallJumpAbility>          WallJumpAbility;
    UPROPERTY(VisibleAnywhere, Category = "Movement")     TObjectPtr<USlideAbility>             SlideAbility;

    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<UWeaponRegistry>           WeaponRegistry;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<UGunWeapon>                GunWeapon;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<USwordWeapon>              SwordWeapon;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<UGunSkill>                 GunSkill;
    UPROPERTY(VisibleAnywhere, Category = "Weapon")       TObjectPtr<USwordSkill>               SwordSkill;

    UPROPERTY(VisibleAnywhere, Category = "Presentation") TObjectPtr<UAnimationPlayerComponent> AnimationPlayer;
    UPROPERTY(VisibleAnywhere, Category = "Presentation") TObjectPtr<UCameraEffectsComponent>   CameraEffects;

    UPROPERTY(VisibleAnywhere, Category = "Health")      TObjectPtr<UHealthComponent>           Health;

    UPROPERTY(EditDefaultsOnly, Category = "HUD")        TSubclassOf<UPlayerHUDWidget>          HUDWidgetClass;
    TObjectPtr<UPlayerHUDWidget>                         HUDWidget;
};