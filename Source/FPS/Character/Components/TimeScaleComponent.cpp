#include "TimeScaleComponent.h"
#include "../PlayerCharacter.h"
#include "GameFramework/WorldSettings.h"

UTimeScaleComponent::UTimeScaleComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bTickEvenWhenPaused = true;
}

void UTimeScaleComponent::Initialize(APlayerCharacter* InOwner)
{
    OwnerCharacter = InOwner;
}

void UTimeScaleComponent::ApplyTimeScale(FTimeScaleParams Params)
{
    bIsActive      = true;
    bIsBlendingOut = false;
    TargetDilation = Params.WorldDilation;
    ActiveDuration = Params.Duration;
    ActiveMode     = Params.Mode;
    ElapsedTime    = 0.f;

    BlendInSpeed  = Params.BlendIn  > 0.f ? (1.f - Params.WorldDilation) / Params.BlendIn  : 9999.f;
    BlendOutSpeed = Params.BlendOut > 0.f ? (1.f - Params.WorldDilation) / Params.BlendOut : 9999.f;

    // Full 모드면 플레이어도 같이 느려짐
    if (ActiveMode == ETimeScaleMode::Full && OwnerCharacter)
        OwnerCharacter->CustomTimeDilation = Params.WorldDilation;
}

void UTimeScaleComponent::ClearTimeScale(float OverrideBlendOut)
{
    if (!bIsActive) return;
    bIsBlendingOut = true;

    if (OverrideBlendOut >= 0.f)
        BlendOutSpeed = OverrideBlendOut > 0.f ? (1.f - TargetDilation) / OverrideBlendOut : 9999.f;
}

void UTimeScaleComponent::SetWorldDilation(float Dilation)
{
    CurrentDilation = Dilation;

    if (GetWorld())
        GetWorld()->GetWorldSettings()->TimeDilation = Dilation;

    // Full 모드면 플레이어 CustomTimeDilation도 역보정해서 정상 속도 유지
    if (ActiveMode == ETimeScaleMode::WorldOnly && OwnerCharacter)
        OwnerCharacter->CustomTimeDilation = 1.f / Dilation;

    OnTimeScaleChanged.Broadcast(Dilation);
}

void UTimeScaleComponent::TickBlend(float DeltaTime)
{
    if (bIsBlendingOut)
    {
        CurrentDilation = FMath::FInterpConstantTo(CurrentDilation, 1.f, DeltaTime, BlendOutSpeed);
        SetWorldDilation(CurrentDilation);

        if (FMath::IsNearlyEqual(CurrentDilation, 1.f, 0.01f))
        {
            bIsActive      = false;
            bIsBlendingOut = false;
            CurrentDilation = 1.f;
            SetWorldDilation(1.f);

            // 플레이어 CustomTimeDilation 복구
            if (OwnerCharacter)
                OwnerCharacter->CustomTimeDilation = 1.f;
        }
    }
    else
    {
        CurrentDilation = FMath::FInterpConstantTo(CurrentDilation, TargetDilation, DeltaTime, BlendInSpeed);
        SetWorldDilation(CurrentDilation);

        // 지속 시간 있으면 자동 해제
        if (ActiveDuration > 0.f)
        {
            ElapsedTime += DeltaTime;
            if (ElapsedTime >= ActiveDuration)
                ClearTimeScale();
        }
    }
}

void UTimeScaleComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bIsActive) TickBlend(FApp::GetDeltaTime());
}