#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/Data/TimeDilationRequest.h"
#include "TimeDilationSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeDilationChanged, float, CurrentDilation);

/**
 * World-scoped time dilation manager.
 *
 * Multiple systems (Focus, Iajutsu, hit-stop, ...) push requests with priorities.
 * The subsystem picks the winning request, blends to it, and applies it to the world.
 *
 * Sources of safety:
 *  - Bad input on PushRequest is rejected, never crashes.
 *  - Bad handle on PopRequest is ignored.
 *  - Destroyed requesters are auto-cleaned every frame via TWeakObjectPtr check.
 *  - Division by zero is guarded with KINDA_SMALL_NUMBER.
 */
UCLASS()
class FPS_API UTimeDilationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /**
     * Push a new request. Returns a handle (>0) on success, 0 on rejection.
     * Caller is expected to keep the handle and call PopRequest later.
     */
    int32 PushRequest(const FTimeDilationRequest& Request);

    /** Release a single request by handle. No-op if handle is invalid. */
    void PopRequest(int32 Handle);

    /** Release every request made by Requester. Safe to call from EndPlay. */
    void PopAllRequestsBy(AActor* Requester);

    UPROPERTY(BlueprintAssignable, Category = "TimeDilation")
    FOnTimeDilationChanged OnTimeDilationChanged;

    UFUNCTION(BlueprintPure, Category = "TimeDilation")
    float GetCurrentDilation() const { return CurrentDilation; }

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    struct FActiveRequest
    {
        int32                  Handle = 0;
        FTimeDilationRequest   Request;
        TWeakObjectPtr<AActor> RequesterWeak;
    };

    void Tick(float DeltaTime);
    void RecomputeTarget();
    void ApplyDilation(float Dilation);
    void CleanupDeadRequesters();

    /**
     * Apply the counter-scale to current WorldOnly requesters and restore anyone who
     * no longer has a request. Without the restore step the inverse scale would stay
     * on the actor forever after its request is popped.
     */
    void UpdateCounterScales(float Dilation);

    TArray<FActiveRequest> ActiveRequests;

    /** Actors currently carrying a counter-scale, so we know who to restore. */
    TArray<TWeakObjectPtr<AActor>> CounterScaledActors;

    int32 NextHandle = 1;

    float CurrentDilation = 1.f;
    float TargetDilation  = 1.f;
    float BlendSpeed      = 0.f;

    FTimerHandle TickTimer;
};