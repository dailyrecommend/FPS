#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Presentation/Interfaces/CameraEffects.h"
#include "Presentation/Data/CameraEffectRequest.h"
#include "CameraEffectsComponent.generated.h"

class UCameraComponent;
class APlayerController;

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UCameraEffectsComponent
    : public UActorComponent
    , public ICameraEffects
{
    GENERATED_BODY()

public:
    UCameraEffectsComponent();

    /**
     * Inject the camera and base eye height. The PlayerController is fetched lazily
     * each frame from the owning pawn — passing it here would be unsafe because
     * possession may not have happened yet at injection time.
     */
    void InjectDependencies(UCameraComponent* InCamera, float InBaseEyeHeight);

    virtual int32 PushFOVOffset_Implementation(float Offset, float InterpSpeed, int32 Priority) override;
    virtual void  PopFOVOffset_Implementation(int32 Handle) override;

    virtual int32 PushRollOffset_Implementation(float RollDegrees, float InterpSpeed, int32 Priority) override;
    virtual void  PopRollOffset_Implementation(int32 Handle) override;

    virtual int32 PushHeightOffset_Implementation(float HeightOffset, float InterpSpeed, int32 Priority) override;
    virtual void  PopHeightOffset_Implementation(int32 Handle) override;

    virtual bool  UpdateRollOffset_Implementation(int32 Handle, float NewRollDegrees) override;

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    int32 Push(TArray<FCameraEffectRequest>& Channel, float Value, float InterpSpeed, int32 Priority);
    void  Pop (TArray<FCameraEffectRequest>& Channel, int32 Handle);
    bool  Update(TArray<FCameraEffectRequest>& Channel, int32 Handle, float NewValue);

    static const FCameraEffectRequest* PickWinner(const TArray<FCameraEffectRequest>& Channel);

    /** Look up the owner's PlayerController. May return nullptr early in game start. */
    APlayerController* ResolveController() const;

    void TickFOV(float DeltaTime);
    void TickRoll(float DeltaTime);
    void TickHeight(float DeltaTime);

    UPROPERTY()
    TWeakObjectPtr<UCameraComponent> Camera;

    float BaseEyeHeight = 64.f;
    float DefaultFOV    = 90.f;

    TArray<FCameraEffectRequest> FOVChannel;
    TArray<FCameraEffectRequest> RollChannel;
    TArray<FCameraEffectRequest> HeightChannel;

    int32 NextHandle = 1;

    float CurrentFOVOffset    = 0.f;
    float CurrentRollOffset   = 0.f;
    float CurrentHeightOffset = 0.f;
};