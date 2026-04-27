#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DashComponent.generated.h"

class APlayerCharacter;

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UDashComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDashComponent();

    void Initialize(APlayerCharacter* InOwner);

    void TryDash();
    void SetMoveInput(FVector2D Input) { LastMoveInput = Input; }

    UFUNCTION(BlueprintCallable) void AddDashChargeImmediate();

    UFUNCTION(BlueprintPure) bool  IsDashing()       const { return bIsDashing; }
    UFUNCTION(BlueprintPure) int32 GetDashCharges()  const { return DashCharges; }
    UFUNCTION(BlueprintPure) int32 GetMaxDashCharges() const { return MaxDashCharges; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void PerformDash();
    void AddDashCharge();
    void TickDash(float DeltaTime);
    void TickChargeRecovery(float DeltaTime);

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float DashSpeed            = 6000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float DashDuration         = 0.15f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float DashChargeDelay      = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float DashChargeInterval   = 1.f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float DashAirMomentumSpeed = 1200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    int32 MaxDashCharges       = 3;

    UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;

    bool      bIsDashing       = false;
    float     DashElapsed      = 0.f;
    int32     DashCharges      = 3;
    float     DashChargeTimer  = 0.f;
    bool      bDashChargeDelay = false;
    float     DashDelayTimer   = 0.f;
    FVector   DashDirection    = FVector::ZeroVector;
    FVector2D LastMoveInput    = FVector2D::ZeroVector;
};