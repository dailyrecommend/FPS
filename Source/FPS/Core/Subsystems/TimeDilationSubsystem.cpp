#include "Core/Subsystems/TimeDilationSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "TimerManager.h"

namespace
{
    constexpr float TickInterval        = 1.f / 60.f;
    constexpr float MaxAllowedDilation  = 10.f;
    constexpr float DefaultBlendOutTime = 0.2f;
    constexpr float MaxBlendSpeed       = 100.f;
}

void UTimeDilationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UWorld* World = GetWorld();
    if (!World) return;

    // Use a TimerManager-driven tick so we run independently of any actor's tick group
    // and remain reliable even when the world's TimeDilation is itself modified.
    World->GetTimerManager().SetTimer(
        TickTimer,
        FTimerDelegate::CreateUObject(this, &UTimeDilationSubsystem::Tick, TickInterval),
        TickInterval,
        true);
}

void UTimeDilationSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
        World->GetTimerManager().ClearTimer(TickTimer);

    ActiveRequests.Empty();
    Super::Deinitialize();
}

int32 UTimeDilationSubsystem::PushRequest(const FTimeDilationRequest& Request)
{
    if (Request.WorldDilation <= KINDA_SMALL_NUMBER) return 0;
    if (Request.WorldDilation > MaxAllowedDilation)  return 0;
    if (Request.BlendIn  < 0.f)                      return 0;
    if (Request.BlendOut < 0.f)                      return 0;

    FActiveRequest Active;
    Active.Handle        = NextHandle++;
    Active.Request       = Request;
    Active.RequesterWeak = Request.Requester;

    ActiveRequests.Add(Active);
    RecomputeTarget();

    return Active.Handle;
}

void UTimeDilationSubsystem::PopRequest(int32 Handle)
{
    if (Handle <= 0) return;

    const int32 Removed = ActiveRequests.RemoveAll(
        [Handle](const FActiveRequest& R) { return R.Handle == Handle; });

    if (Removed > 0) RecomputeTarget();
}

void UTimeDilationSubsystem::PopAllRequestsBy(AActor* Requester)
{
    if (!Requester) return;

    const int32 Removed = ActiveRequests.RemoveAll(
        [Requester](const FActiveRequest& R) { return R.RequesterWeak.Get() == Requester; });

    if (Removed > 0) RecomputeTarget();
}

void UTimeDilationSubsystem::CleanupDeadRequesters()
{
    ActiveRequests.RemoveAll(
        [](const FActiveRequest& R) { return !R.RequesterWeak.IsValid(); });
}

void UTimeDilationSubsystem::RecomputeTarget()
{
    CleanupDeadRequesters();

    if (ActiveRequests.Num() == 0)
    {
        TargetDilation = 1.f;
        const float Distance = FMath::Abs(1.f - CurrentDilation);
        BlendSpeed = (DefaultBlendOutTime > KINDA_SMALL_NUMBER)
            ? Distance / DefaultBlendOutTime
            : MaxBlendSpeed;
        return;
    }

    const FActiveRequest* Top = nullptr;
    for (const FActiveRequest& R : ActiveRequests)
    {
        const bool bHigherPriority = !Top || R.Request.Priority > Top->Request.Priority;
        const bool bTieStronger    = Top
            && R.Request.Priority == Top->Request.Priority
            && R.Request.WorldDilation < Top->Request.WorldDilation;

        if (bHigherPriority || bTieStronger)
            Top = &R;
    }

    if (!Top)
    {
        TargetDilation = 1.f;
        return;
    }

    TargetDilation = Top->Request.WorldDilation;

    const bool  bSlowingDown = TargetDilation < CurrentDilation;
    const float BlendTime    = bSlowingDown ? Top->Request.BlendIn : Top->Request.BlendOut;
    const float Distance     = FMath::Abs(TargetDilation - CurrentDilation);

    BlendSpeed = (BlendTime > KINDA_SMALL_NUMBER)
        ? Distance / BlendTime
        : MaxBlendSpeed;
}

void UTimeDilationSubsystem::Tick(float DeltaTime)
{
    CleanupDeadRequesters();

    if (FMath::IsNearlyEqual(CurrentDilation, TargetDilation, 0.001f))
    {
        if (CurrentDilation != TargetDilation)
        {
            CurrentDilation = TargetDilation;
            ApplyDilation(CurrentDilation);
        }
        return;
    }

    CurrentDilation = FMath::FInterpConstantTo(CurrentDilation, TargetDilation, DeltaTime, BlendSpeed);
    ApplyDilation(CurrentDilation);
}

void UTimeDilationSubsystem::ApplyDilation(float Dilation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (AWorldSettings* Settings = World->GetWorldSettings())
        Settings->TimeDilation = Dilation;

    for (const FActiveRequest& R : ActiveRequests)
    {
        AActor* Actor = R.RequesterWeak.Get();
        if (!Actor) continue;

        if (R.Request.Mode == ETimeScaleMode::WorldOnly)
        {
            // Counter-scale the requester so they appear to move at normal speed.
            Actor->CustomTimeDilation = (Dilation > KINDA_SMALL_NUMBER) ? (1.f / Dilation) : 1.f;
        }
        else
        {
            Actor->CustomTimeDilation = 1.f;
        }
    }

    OnTimeDilationChanged.Broadcast(Dilation);
}