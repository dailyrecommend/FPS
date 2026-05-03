#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/WeaponHitResult.h"
#include "IajutsuComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;
class USwordComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIajutsuStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIajutsuEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIajutsuCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIajutsuHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UIajutsuComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UIajutsuComponent();

    void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera, USwordComponent* InSword);

    // Called on skill input press — begins hold and slows time
    void StartHold();

    // Called on skill input release — performs dash if not expired
    void EndHold();

    UPROPERTY(BlueprintAssignable) FOnIajutsuStarted   OnIajutsuStarted;
    UPROPERTY(BlueprintAssignable) FOnIajutsuEnded     OnIajutsuEnded;
    UPROPERTY(BlueprintAssignable) FOnIajutsuCancelled OnIajutsuCancelled;
    UPROPERTY(BlueprintAssignable) FOnIajutsuHit       OnHit;

    UFUNCTION(BlueprintPure) bool  IsHolding()            const { return bIsHolding; }
    UFUNCTION(BlueprintPure) bool  IsStunned()            const { return bIsStunned; }
    UFUNCTION(BlueprintPure) bool  CanIajutsu()           const;
    UFUNCTION(BlueprintPure) float GetCooldownRemaining() const { return CooldownRemaining; }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void     PerformDash();
    void     Cancel();
    FVector  CalculateDestination() const;
    void     HitActorsAlongPath(const FVector& Start, const FVector& End);
    void     BroadcastHit(AActor* HitActor, const FVector& Location, const FVector& Normal);
    void     ApplyTimeScale();
    void     ClearTimeScale();
    void     PlayMontage(UAnimMontage* Montage);
    void     TickHold(float DeltaTime);
    void     TickDash(float DeltaTime);
    void     TickStun(float DeltaTime);
    void     TickCooldown(float DeltaTime);

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuDamage       = 200.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuDistance     = 1500.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuCooldown     = 6.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuDashDuration = 0.1f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuStunDuration = 0.3f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         HoldMaxDuration     = 3.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         SlowWorldDilation   = 0.15f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") UAnimMontage* HoldMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") UAnimMontage* DashMontage;

    UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
    UPROPERTY() UCameraComponent* Camera         = nullptr;
    UPROPERTY() USwordComponent*  Sword          = nullptr;

    // Hold state
    bool  bIsHolding    = false;
    float HoldElapsed   = 0.f;

    // Dash state
    bool    bIsDashing      = false;
    float   DashElapsed     = 0.f;
    FVector DashStart       = FVector::ZeroVector;
    FVector DashDestination = FVector::ZeroVector;

    // Post-dash stun
    bool  bIsStunned  = false;
    float StunElapsed = 0.f;

    float CooldownRemaining = 0.f;
};