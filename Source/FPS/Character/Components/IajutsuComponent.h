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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIajutsuHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UIajutsuComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UIajutsuComponent();

    void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera, USwordComponent* InSword);

    void StartIajutsu();

    UPROPERTY(BlueprintAssignable) FOnIajutsuStarted OnIajutsuStarted;
    UPROPERTY(BlueprintAssignable) FOnIajutsuEnded   OnIajutsuEnded;
    UPROPERTY(BlueprintAssignable) FOnIajutsuHit     OnHit;

    UFUNCTION(BlueprintPure) bool  IsIajutsu()            const { return bIsIajutsu; }
    UFUNCTION(BlueprintPure) bool  CanIajutsu()           const;
    UFUNCTION(BlueprintPure) float GetCooldownRemaining() const { return CooldownRemaining; }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void PerformIajutsu();
    void EndIajutsu();
    void TickDash(float DeltaTime);
    void TickCooldown(float DeltaTime);
    void BroadcastHit(AActor* HitActor, const FVector& Location, const FVector& Normal);
    void PlayMontage(UAnimMontage* Montage);

    UFUNCTION()
    void OnHitboxOverlapDuringIajutsu(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                      bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuDamage       = 200.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuSpeed        = 3000.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuDistance     = 1500.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuCooldown     = 6.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") float         IajutsuExitMomentum = 800.f;
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu") UAnimMontage* IajutsuMontage;

    UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
    UPROPERTY() UCameraComponent* Camera         = nullptr;
    UPROPERTY() USwordComponent*  Sword          = nullptr;

    bool    bIsIajutsu        = false;
    float   IajutsuElapsed    = 0.f;
    float   CooldownRemaining = 0.f;
    FVector DashDirection     = FVector::ZeroVector;

    TSet<AActor*> HitActors;
};