#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UCameraManagerComponent;
class UPlayerCharacterInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class FPS_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();
    virtual void Landed(const FHitResult& Hit) override;

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UPlayerCharacterInputConfig* InputConfig;

private:
    //= Movement & Look
    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);
    void Input_JumpStarted();
    void Input_JumpCompleted();

    void BindInputActions(UInputComponent* PlayerInputComponent);
    void RegisterInputMappingContext();

    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraManagerComponent* CameraManager;

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

    //= Glissando
    void Input_SlideStarted();
    void Input_SlideCompleted();
    void StartGlissando();
    void EndGlissando();
    void TickGlissando(float DeltaTime);
    bool CanGlissando() const;

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditDefaultsOnly, Category="Glissando")
    float GlissandoBoostSpeed = 1600.f;

    UPROPERTY(EditDefaultsOnly, Category="Glissando")
    float GlissandoLateralControl = 4.f;

    UPROPERTY(EditDefaultsOnly, Category="Glissando")
    float GlissandoCameraHeight = -40.f;

    UPROPERTY(EditDefaultsOnly, Category="Glissando")
    float GlissandoCameraRoll = 3.f;

    UPROPERTY(EditDefaultsOnly, Category="Glissando")
    float GlissandoMinSpeed = 100.f;

    UPROPERTY(EditDefaultsOnly, Category="Glissando")
    float GlissandoCameraRollInterpSpeed = 8.f;

    bool      bIsGlissando       = false;
    bool      bWantsGlissando    = false;
    float     CurrentCameraRoll  = 0.f;
    FVector   GlissandoDirection = FVector::ZeroVector;
    FVector2D CurrentMoveInput   = FVector2D::ZeroVector;

    //= Dash
    void Input_DashStarted();
    void PerformDash();
    void TickDash(float DeltaTime);
    void AddDashCharge();

    UFUNCTION(BlueprintCallable)
    void AddDashChargeImmediate();

    UPROPERTY(EditDefaultsOnly, Category="Dash")
    float DashSpeed = 6000.f;

    UPROPERTY(EditDefaultsOnly, Category="Dash")
    float DashDuration = 0.15f;

    UPROPERTY(EditDefaultsOnly, Category="Dash")
    float DashChargeDelay = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category="Dash")
    float DashChargeInterval = 1.f;

    UPROPERTY(EditDefaultsOnly, Category="Dash")
    int32 MaxDashCharges = 3;

    bool  bIsDashing       = false;
    float DashElapsed      = 0.f;
    int32 DashCharges      = 3;
    float DashChargeTimer  = 0.f;
    bool  bDashChargeDelay = false;
    float DashDelayTimer   = 0.f;
    FVector DashDirection  = FVector::ZeroVector;
};