#pragma once
#include "CoreMinimal.h"
#include "Movement/Base/AbilityBase.h"
#include "DashAbility.generated.h"

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UDashAbility : public UAbilityBase
{
    GENERATED_BODY()

public:
    UDashAbility();

    UFUNCTION(BlueprintCallable) void  AddDashChargeImmediate();
    UFUNCTION(BlueprintPure)     int32 GetDashCharges()    const { return DashCharges; }
    UFUNCTION(BlueprintPure)     int32 GetMaxDashCharges() const { return MaxDashCharges; }

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Interactions")
    FName SlamAbilityIdToCancel = TEXT("Slam");

protected:
    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    virtual EActivationResult OnTryActivate(const FAbilityContext& Context) override;
    virtual void              OnDeactivate() override;
    virtual bool              CheckPreconditions(const FAbilityContext& Context) const override;

private:
    void TickDash(float DeltaTime);
    void TickChargeRecovery(float DeltaTime);
    void AddDashCharge();
    void RequestSlamCancelIfActive() const;

    UPROPERTY(EditDefaultsOnly, Category = "Dash") float DashSpeed            = 6000.f;
    UPROPERTY(EditDefaultsOnly, Category = "Dash") float DashDuration         = 0.15f;
    UPROPERTY(EditDefaultsOnly, Category = "Dash") float DashChargeDelay      = 0.5f;
    UPROPERTY(EditDefaultsOnly, Category = "Dash") float DashChargeInterval   = 1.f;
    UPROPERTY(EditDefaultsOnly, Category = "Dash") float DashAirMomentumSpeed = 1200.f;
    UPROPERTY(EditDefaultsOnly, Category = "Dash") int32 MaxDashCharges       = 3;

    int32   DashCharges      = 3;
    float   DashElapsed      = 0.f;
    float   DashChargeTimer  = 0.f;
    float   DashDelayTimer   = 0.f;
    bool    bDashChargeDelay = false;
    FVector DashDirection    = FVector::ZeroVector;
};