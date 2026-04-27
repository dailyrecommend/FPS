#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UCameraManagerComponent;
class UGlissandoComponent;
class UDashComponent;
class UPlayerCharacterInputConfig;
class UWallJumpComponent;
class USlamComponent;
class UGunComponent;
class UFocusComponent;
class UTimeScaleComponent;
class UWeaponSwapComponent;
class UJumpComponent;
struct FInputActionValue;

UCLASS()
class FPS_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    //~ Getters - Movement defaults
    UFUNCTION(BlueprintPure) float GetDefaultCameraHeight()        const { return DefaultCameraHeight; }
    UFUNCTION(BlueprintPure) float GetDefaultGroundFriction()      const { return DefaultGroundFriction; }
    UFUNCTION(BlueprintPure) float GetDefaultBrakingDeceleration() const { return DefaultBrakingDeceleration; }
    UFUNCTION(BlueprintPure) float GetDefaultFOV()                 const { return DefaultFOV; }

    //~ Getters - Components
    UFUNCTION(BlueprintPure) USkeletalMeshComponent* GetArmsMesh()          const { return ArmsMesh; }
    UFUNCTION(BlueprintPure) UTimeScaleComponent*    GetTimeScaleComponent() const { return TimeScale; }
    UFUNCTION(BlueprintPure) UFocusComponent*        GetFocusComponent()     const { return Focus; }
    UFUNCTION(BlueprintPure) USkeletalMeshComponent* GetGunMesh()            const { return GunMesh; }
    UFUNCTION(BlueprintPure) USkeletalMeshComponent* GetSwordMesh()          const { return SwordMesh; }

    //~ Look sensitivity (modulated by Focus state)
    UFUNCTION(BlueprintPure) float GetLookSensitivityMultiplier() const;

    //~ Current move input, read by movement components
    FVector2D CurrentMoveInput = FVector2D::ZeroVector;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Landed(const FHitResult& Hit) override;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UPlayerCharacterInputConfig* InputConfig;

private:
    //~ Input handlers
    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);
    void Input_JumpStarted();
    void Input_JumpCompleted();
    void Input_SlideStarted();
    void Input_SlideCompleted();
    void Input_DashStarted();
    void Input_SlamStarted();
    void Input_AttackStarted();
    void Input_WeaponSkillStarted();
    void Input_WeaponSkillCompleted();
    void Input_WeaponSwapGun();
    void Input_WeaponSwapSword();
    void Input_WeaponSwapScroll(const FInputActionValue& Value);

    void BindInputActions(UInputComponent* PlayerInputComponent);
    void RegisterInputMappingContext();
    void InitializeComponents();
    void SetupMovementDefaults();

    //~ Components - Camera
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UCameraManagerComponent* CameraManager;

    //~ Components - Movement
    UPROPERTY(VisibleAnywhere, Category = "Movement")
    UGlissandoComponent* Glissando;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    UDashComponent* Dash;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    UWallJumpComponent* WallJump;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    USlamComponent* Slam;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    UJumpComponent* JumpComp;

    //~ Components - Weapon
    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    UGunComponent* Gun;

    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    UFocusComponent* Focus;

    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    UWeaponSwapComponent* WeaponSwap;

    //~ Components - Mesh
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    USkeletalMeshComponent* ArmsMesh;

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    USkeletalMeshComponent* GunMesh;

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    USkeletalMeshComponent* SwordMesh;

    //~ Components - System
    UPROPERTY(VisibleAnywhere, Category = "System")
    UTimeScaleComponent* TimeScale;

    //~ Movement config
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultMaxWalkSpeed = 800.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultJumpZVelocity = 800.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultGroundFriction = 8.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultBrakingDeceleration = 2048.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultAirControl = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float DefaultFallingLateralFriction = 0.3f;

    //~ Camera config
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float DefaultCameraHeight = 64.f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float DefaultFOV = 90.f;
};