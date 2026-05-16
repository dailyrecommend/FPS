#include "Weapons/Implementations/GunSkill/GunSkill.h"
#include "Weapons/Implementations/Gun/GunWeapon.h"
#include "Core/Subsystems/TimeDilationSubsystem.h"
#include "Core/Data/TimeDilationRequest.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/App.h"

UGunSkill::UGunSkill()
{
}

bool UGunSkill::OnStartHold()
{
    Elapsed = 0.f;

    PlayMontage(ChargeMontage);
    FOVHandle = PushFOVOffset(FOVOffset, FOVInterpSpeed, FOVPriority);

    ACharacter* Owner = GetOwnerSafe();
    if (Owner && Owner->GetWorld())
    {
        Owner->GetWorld()->GetTimerManager().SetTimer(
            StartDelayTimer,
            FTimerDelegate::CreateUObject(this, &UGunSkill::RequestTimeDilationDelayed),
            FMath::Max(StartDelay, 0.001f),
            false);
    }

    OnGunSkillStateChanged.Broadcast(true);
    return true;
}

void UGunSkill::OnEndHold()
{
    ACharacter* Owner = GetOwnerSafe();
    if (Owner && Owner->GetWorld())
        Owner->GetWorld()->GetTimerManager().ClearTimer(StartDelayTimer);

    StartCooldown(CooldownDuration);

    ReleaseTimeDilation();
    ReleaseTimeDilation();
    PopFOVOffset(FOVHandle);
    FOVHandle = 0;

    StopMontage(ChargeMontage);
    PlayMontage(FireMontage);

    if (UGunWeapon* G = Gun.Get())
        G->FireChargedShot(ChargedDamage, FireLockout);

    OnGunSkillStateChanged.Broadcast(false);
}

void UGunSkill::OnCancel()
{
    ACharacter* Owner = GetOwnerSafe();
    if (Owner && Owner->GetWorld())
        Owner->GetWorld()->GetTimerManager().ClearTimer(StartDelayTimer);

    ReleaseTimeDilation();
    PopFOVOffset(FOVHandle);
    FOVHandle = 0;

    StopMontage(ChargeMontage);
    OnGunSkillStateChanged.Broadcast(false);
}

void UGunSkill::EndPlay(EEndPlayReason::Type Reason)
{
    ReleaseTimeDilation();
    PopFOVOffset(FOVHandle);
    FOVHandle = 0;

    if (ACharacter* Owner = GetOwnerSafe(); Owner && Owner->GetWorld())
        Owner->GetWorld()->GetTimerManager().ClearTimer(StartDelayTimer);

    Super::EndPlay(Reason);
}

void UGunSkill::RequestTimeDilationDelayed()
{
    if (!bIsActive) return;
    RequestTimeDilation();
}

void UGunSkill::RequestTimeDilation()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    UTimeDilationSubsystem* TimeSys = Owner->GetWorld()->GetSubsystem<UTimeDilationSubsystem>();
    if (!TimeSys) return;

    if (TimeDilationHandle != 0)
    {
        TimeSys->PopRequest(TimeDilationHandle);
        TimeDilationHandle = 0;
    }

    FTimeDilationRequest Request;
    Request.WorldDilation = WorldDilation;
    Request.Mode          = ETimeScaleMode::Full;
    Request.BlendIn       = DilationBlendIn;
    Request.BlendOut      = DilationBlendOut;
    Request.Priority      = DilationPriority;
    Request.Requester     = Owner;

    TimeDilationHandle = TimeSys->PushRequest(Request);
}

void UGunSkill::ReleaseTimeDilation()
{
    if (TimeDilationHandle == 0) return;

    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) { TimeDilationHandle = 0; return; }

    if (UTimeDilationSubsystem* TimeSys = Owner->GetWorld()->GetSubsystem<UTimeDilationSubsystem>())
        TimeSys->PopRequest(TimeDilationHandle);

    TimeDilationHandle = 0;
}

void UGunSkill::TickDuration(float UnscaledDelta)
{
    if (!bIsActive) return;
    Elapsed += UnscaledDelta;
    if (Elapsed >= MaxDuration)
        EndHold_Implementation();
}

void UGunSkill::TickComponent(float DeltaTime, ELevelTick TickType,
                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const float UnscaledDelta = FApp::GetDeltaTime();
    TickDuration(UnscaledDelta);
    TickCooldown(UnscaledDelta);
}