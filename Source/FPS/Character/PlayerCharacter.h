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
struct FInputActionValue;

UCLASS()
class FPS_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    UFUNCTION(BlueprintPure) float GetDefaultCameraHeight()        const { return DefaultCameraHeight; }
    UFUNCTION(BlueprintPure) float GetDefaultGroundFriction()      const { return DefaultGroundFriction; }
    UFUNCTION(BlueprintPure) float GetDefaultBrakingDeceleration() const { return DefaultBrakingDeceleration; }
    UFUNCTION(BlueprintPure) USkeletalMeshComponent* GetArmsMesh() const { return ArmsMesh; }

    
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Landed(const FHitResult& Hit) override;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UPlayerCharacterInputConfig* InputConfig;

private:
    //= Input
    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);
    void Input_JumpStarted();
    void Input_JumpCompleted();
    void Input_SlideStarted();
    void Input_SlideCompleted();
    void Input_DashStarted();
    void Input_SlamStarted();
    void Input_AttackStarted();
    void TickCoyoteTime(float DeltaTime);
    void TickJumpBuffer(float DeltaTime);
    bool CanCoyoteJump() const;
    

    void BindInputActions(UInputComponent* PlayerInputComponent);
    void RegisterInputMappingContext();

    //= Components
    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraManagerComponent* CameraManager;

    UPROPERTY(VisibleAnywhere, Category="Movement")
    UGlissandoComponent* Glissando;

    UPROPERTY(VisibleAnywhere, Category="Movement")
    UDashComponent* Dash;

    //= Movement Config
    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float DefaultMaxWalkSpeed = 800.f;

    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float DefaultJumpZVelocity = 800.f;

    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float DefaultGroundFriction = 8.f;

    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float DefaultBrakingDeceleration = 2048.f;

    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float DefaultAirControl = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float DefaultFallingLateralFriction = 0.3f;

    //= Camera Config
    UPROPERTY(EditDefaultsOnly, Category="Camera")
    float DefaultCameraHeight = 64.f;

    //= Coyote Time
    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float CoyoteTimeThreshold = 0.15f;

    float CoyoteTimeCounter = 0.f;
    bool  bWasGrounded      = false;
    bool  bCoyoteJumpUsed   = false;

    //= Input Buffer
    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float JumpBufferDuration = 0.15f;

    float JumpBufferCounter = 0.f;
    bool  bJumpBuffered     = false;

    //= WallJump
    UPROPERTY(VisibleAnywhere, Category="Movement")
    UWallJumpComponent* WallJump;

    //= Slam
    UPROPERTY(VisibleAnywhere, Category="Movement")
    USlamComponent* Slam;

    //= Weapon
    UPROPERTY(VisibleAnywhere, Category="Weapon")
    UGunComponent* Gun;

    //= ArmMesh
    UPROPERTY(VisibleAnywhere, Category="Mesh")
    USkeletalMeshComponent* ArmsMesh;

    
};