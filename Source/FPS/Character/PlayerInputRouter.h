#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInputRouter.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UAbilityRegistry;
class UWeaponRegistry;
class UInputComponent;
class UPlayerCharacterInputConfig;
struct FInputActionValue;

/**
 * Routes Enhanced Input actions to domain registries.
 *
 * Pulled out of PlayerCharacter so the character class becomes a pure assembly point —
 * the router knows nothing about concrete abilities or weapons, only registries and ids.
 *
 * Adding a new ability or weapon requires no changes here: register it under a known id
 * and bind the input asset accordingly.
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UPlayerInputRouter : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerInputRouter();

    void InjectDependencies(ACharacter* InOwner, UAbilityRegistry* InAbilityRegistry, UWeaponRegistry* InWeaponRegistry);
    void SetInputConfig(UPlayerCharacterInputConfig* InConfig) { InputConfig = InConfig; }

    /** Bind every input action to the appropriate handler. Called by PlayerCharacter::SetupPlayerInputComponent. */
    void BindInputActions(UInputComponent* PlayerInputComponent);

    UFUNCTION(BlueprintPure, Category = "Input")
    FVector2D GetCurrentMoveInput() const { return CurrentMoveInput; }

private:
    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);

    void Input_JumpStarted();
    void Input_DashStarted();
    void Input_SlamStarted();
    void Input_SlideStarted();
    void Input_SlideCompleted();

    void Input_AttackStarted();
    void Input_WeaponSkillStarted();
    void Input_WeaponSkillCompleted();

    void Input_WeaponSwapGun();
    void Input_WeaponSwapSword();
    void Input_WeaponSwapScroll(const FInputActionValue& Value);
    void Input_WeaponSwapNext();
    void Input_WeaponSwapPrev();

    /** Build the FAbilityContext snapshot from current owner state. */
    struct FAbilityContext BuildContext() const;

    UPROPERTY() TWeakObjectPtr<ACharacter>            OwnerCharacter;
    UPROPERTY() TWeakObjectPtr<UAbilityRegistry>      AbilityRegistry;
    UPROPERTY() TWeakObjectPtr<UWeaponRegistry>       WeaponRegistry;
    UPROPERTY() TObjectPtr<UPlayerCharacterInputConfig> InputConfig;

    FVector2D CurrentMoveInput = FVector2D::ZeroVector;
};