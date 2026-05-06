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
}