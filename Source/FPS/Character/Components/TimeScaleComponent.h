#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimeScaleComponent.generated.h"

class APlayerCharacter;

UENUM(BlueprintType)
enum class ETimeScaleMode : uint8
{
    WorldOnly,  // 세계만 슬로우, 플레이어 정상
    Full,       // 플레이어 + 세계 동시 슬로우
};

USTRUCT(BlueprintType)
struct FTimeScaleParams
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly) ETimeScaleMode Mode         = ETimeScaleMode::WorldOnly;
    UPROPERTY(EditDefaultsOnly) float          WorldDilation = 0.2f;
    UPROPERTY(EditDefaultsOnly) float          Duration      = 0.f;   // 0이면 수동 해제
    UPROPERTY(EditDefaultsOnly) float          BlendIn       = 0.1f;
    UPROPERTY(EditDefaultsOnly) float          BlendOut      = 0.3f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeScaleChanged, float, WorldDilation);

UCLASS(ClassGroup=Custom, meta=(BlueprintSpawnableComponent))
class FPS_API UTimeScaleComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTimeScaleComponent();

    void Initialize(APlayerCharacter* InOwner);

    void ApplyTimeScale(FTimeScaleParams Params);
    void ClearTimeScale(float OverrideBlendOut = -1.f);

    UPROPERTY(BlueprintAssignable) FOnTimeScaleChanged OnTimeScaleChanged;

    UFUNCTION(BlueprintPure) bool  IsTimeScaleActive()   const { return bIsActive; }
    UFUNCTION(BlueprintPure) float GetCurrentDilation() const { return CurrentDilation; }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void SetWorldDilation(float Dilation);
    void TickBlend(float DeltaTime);

    UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;

    bool             bIsActive        = false;
    bool             bIsBlendingOut   = false;
    float            CurrentDilation  = 1.f;
    float            TargetDilation   = 1.f;
    float            BlendInSpeed     = 0.f;
    float            BlendOutSpeed    = 0.f;
    float            ActiveDuration   = 0.f;
    float            ElapsedTime      = 0.f;
    ETimeScaleMode   ActiveMode       = ETimeScaleMode::WorldOnly;
};