#include "Presentation/Components/CameraEffectsComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UCameraEffectsComponent::UCameraEffectsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCameraEffectsComponent::InjectDependencies(UCameraComponent* InCamera, float InBaseEyeHeight)
{
    Camera        = InCamera;
    BaseEyeHeight = InBaseEyeHeight;

    if (UCameraComponent* Cam = Camera.Get())
        DefaultFOV = Cam->FieldOfView;
}

APlayerController* UCameraEffectsComponent::ResolveController() const
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return nullptr;
    return Cast<APlayerController>(OwnerPawn->GetController());
}

int32 UCameraEffectsComponent::Push(TArray<FCameraEffectRequest>& Channel, float Value, float InterpSpeed, int32 Priority)
{
    if (InterpSpeed < 0.f) return 0;

    FCameraEffectRequest Request;
    Request.Handle      = NextHandle++;
    Request.Value       = Value;
    Request.InterpSpeed = InterpSpeed;
    Request.Priority    = Priority;

    Channel.Add(Request);
    return Request.Handle;
}

void UCameraEffectsComponent::Pop(TArray<FCameraEffectRequest>& Channel, int32 Handle)
{
    if (Handle <= 0) return;
    Channel.RemoveAll([Handle](const FCameraEffectRequest& R) { return R.Handle == Handle; });
}

bool UCameraEffectsComponent::Update(TArray<FCameraEffectRequest>& Channel, int32 Handle, float NewValue)
{
    if (Handle <= 0) return false;

    for (FCameraEffectRequest& R : Channel)
    {
        if (R.Handle == Handle)
        {
            R.Value = NewValue;
            return true;
        }
    }
    return false;
}

const FCameraEffectRequest* UCameraEffectsComponent::PickWinner(const TArray<FCameraEffectRequest>& Channel)
{
    const FCameraEffectRequest* Winner = nullptr;

    for (const FCameraEffectRequest& R : Channel)
    {
        if (!Winner)
        {
            Winner = &R;
            continue;
        }
        if (R.Priority > Winner->Priority)
        {
            Winner = &R;
            continue;
        }
        if (R.Priority == Winner->Priority && FMath::Abs(R.Value) > FMath::Abs(Winner->Value))
        {
            Winner = &R;
        }
    }
    return Winner;
}

int32 UCameraEffectsComponent::PushFOVOffset_Implementation(float Offset, float InterpSpeed, int32 Priority)
{
    return Push(FOVChannel, Offset, InterpSpeed, Priority);
}

void UCameraEffectsComponent::PopFOVOffset_Implementation(int32 Handle)
{
    Pop(FOVChannel, Handle);
}

int32 UCameraEffectsComponent::PushRollOffset_Implementation(float RollDegrees, float InterpSpeed, int32 Priority)
{
    return Push(RollChannel, RollDegrees, InterpSpeed, Priority);
}

void UCameraEffectsComponent::PopRollOffset_Implementation(int32 Handle)
{
    Pop(RollChannel, Handle);
}

int32 UCameraEffectsComponent::PushHeightOffset_Implementation(float HeightOffset, float InterpSpeed, int32 Priority)
{
    return Push(HeightChannel, HeightOffset, InterpSpeed, Priority);
}

void UCameraEffectsComponent::PopHeightOffset_Implementation(int32 Handle)
{
    Pop(HeightChannel, Handle);
}

bool UCameraEffectsComponent::UpdateRollOffset_Implementation(int32 Handle, float NewRollDegrees)
{
    return Update(RollChannel, Handle, NewRollDegrees);
}

void UCameraEffectsComponent::TickFOV(float DeltaTime)
{
    UCameraComponent* Cam = Camera.Get();
    if (!Cam) return;

    const FCameraEffectRequest* Winner = PickWinner(FOVChannel);
    const float TargetOffset = Winner ? Winner->Value       : 0.f;
    const float Speed        = Winner ? Winner->InterpSpeed : 8.f;

    CurrentFOVOffset = FMath::FInterpTo(CurrentFOVOffset, TargetOffset, DeltaTime, Speed);
    Cam->SetFieldOfView(DefaultFOV + CurrentFOVOffset);
}

void UCameraEffectsComponent::TickRoll(float DeltaTime)
{
    APlayerController* PC = ResolveController();
    if (!PC) return;

    const FCameraEffectRequest* Winner = PickWinner(RollChannel);
    const float TargetOffset = Winner ? Winner->Value       : 0.f;
    const float Speed        = Winner ? Winner->InterpSpeed : 8.f;

    CurrentRollOffset = FMath::FInterpTo(CurrentRollOffset, TargetOffset, DeltaTime, Speed);

    FRotator ControlRot = PC->GetControlRotation();
    ControlRot.Roll     = CurrentRollOffset;
    PC->SetControlRotation(ControlRot);
}

void UCameraEffectsComponent::TickHeight(float DeltaTime)
{
    UCameraComponent* Cam = Camera.Get();
    if (!Cam) return;

    const FCameraEffectRequest* Winner = PickWinner(HeightChannel);
    const float TargetOffset = Winner ? Winner->Value       : 0.f;
    const float Speed        = Winner ? Winner->InterpSpeed : 8.f;

    CurrentHeightOffset = FMath::FInterpTo(CurrentHeightOffset, TargetOffset, DeltaTime, Speed);
    Cam->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight + CurrentHeightOffset));
}

void UCameraEffectsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickFOV(DeltaTime);
    TickRoll(DeltaTime);
    TickHeight(DeltaTime);
    TickShake(DeltaTime);
    TickKickback(DeltaTime);
}

void UCameraEffectsComponent::TriggerShake_Implementation(
    float Magnitude, float Duration, float Frequency)
{
    ShakeMagnitude = Magnitude;
    ShakeDuration  = Duration;
    ShakeFrequency = Frequency;
    ShakeElapsed   = 0.f;
    ShakeTimer     = 0.f;
}

void UCameraEffectsComponent::TickShake(float DeltaTime)
{
    if (ShakeDuration <= 0.f || ShakeElapsed >= ShakeDuration)
    {
        if (LastShakePitch != 0.f || LastShakeYaw != 0.f)
        {
            APlayerController* PC = ResolveController();
            if (PC)
            {
                FRotator ControlRot = PC->GetControlRotation();
                ControlRot.Pitch   -= LastShakePitch;
                ControlRot.Yaw     -= LastShakeYaw;
                PC->SetControlRotation(ControlRot);
            }
            LastShakePitch = 0.f;
            LastShakeYaw   = 0.f;
        }
        return;
    }

    ShakeElapsed += DeltaTime;

    APlayerController* PC = ResolveController();
    if (!PC) return;

    const float Decay    = 1.f - FMath::Clamp(ShakeElapsed / ShakeDuration, 0.f, 1.f);
    const float NewPitch = FMath::RandRange(-ShakeMagnitude, ShakeMagnitude) * Decay;
    const float NewYaw   = FMath::RandRange(-ShakeMagnitude, ShakeMagnitude) * Decay * 0.5f;

    FRotator ControlRot = PC->GetControlRotation();
    ControlRot.Pitch   -= LastShakePitch;
    ControlRot.Pitch   += NewPitch;
    ControlRot.Yaw     -= LastShakeYaw;
    ControlRot.Yaw     += NewYaw;
    PC->SetControlRotation(ControlRot);

    LastShakePitch = NewPitch;
    LastShakeYaw   = NewYaw;
}

void UCameraEffectsComponent::TriggerKickback_Implementation(
    float PitchAmount, float YawAmount, float Duration)
{
    KickbackPitch    = PitchAmount;
    KickbackYaw      = YawAmount;
    KickbackDuration = Duration;
    KickbackElapsed  = 0.f;
    LastKickPitch    = 0.f;
    LastKickYaw      = 0.f;
}

void UCameraEffectsComponent::TickKickback(float DeltaTime)
{
    if (KickbackDuration <= 0.f || KickbackElapsed >= KickbackDuration)
    {
        if (LastKickPitch != 0.f || LastKickYaw != 0.f)
        {
            APlayerController* PC = ResolveController();
            if (PC)
            {
                FRotator ControlRot = PC->GetControlRotation();
                ControlRot.Pitch   -= LastKickPitch;
                ControlRot.Yaw     -= LastKickYaw;
                PC->SetControlRotation(ControlRot);
            }
            LastKickPitch = 0.f;
            LastKickYaw   = 0.f;
        }
        return;
    }

    KickbackElapsed += DeltaTime;

    APlayerController* PC = ResolveController();
    if (!PC) return;

    const float Decay        = 1.f - FMath::Clamp(KickbackElapsed / KickbackDuration, 0.f, 1.f);
    const float TargetPitch  = KickbackPitch * Decay;
    const float TargetYaw    = KickbackYaw   * Decay;

    FRotator ControlRot = PC->GetControlRotation();
    ControlRot.Pitch   -= LastKickPitch;
    ControlRot.Pitch   += TargetPitch;
    ControlRot.Yaw     -= LastKickYaw;
    ControlRot.Yaw     += TargetYaw;
    PC->SetControlRotation(ControlRot);

    LastKickPitch = TargetPitch;
    LastKickYaw   = TargetYaw;
}