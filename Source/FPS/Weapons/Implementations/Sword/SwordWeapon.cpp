#include "Weapons/Implementations/Sword/SwordWeapon.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Combat/Interfaces/Damageable.h"
#include "Presentation/Components/AnimationPlayerComponent.h"
#include "Presentation/Interfaces/AnimationPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

USwordWeapon::USwordWeapon()
{
    WeaponId = TEXT("Sword");
    Cooldown = 0.f;

    PrimaryComponentTick.bCanEverTick = true;

    SlashHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("SlashHitbox"));
    SlashHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SlashHitbox->SetCollisionObjectType(ECC_WorldDynamic);
    SlashHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
    SlashHitbox->SetCollisionResponseToChannel(ECC_Pawn,         ECR_Overlap);
    SlashHitbox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void USwordWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (SlashHitbox)
        SlashHitbox->OnComponentBeginOverlap.AddDynamic(this, &USwordWeapon::OnHitboxOverlap);
}

void USwordWeapon::AttachAnimationPlayer(TScriptInterface<IAnimationPlayer> InPlayer)
{
    Super::AttachAnimationPlayer(InPlayer);

    UAnimationPlayerComponent* AnimPlayer = Cast<UAnimationPlayerComponent>(InPlayer.GetObject());
    if (!AnimPlayer) return;

    USkeletalMeshComponent* Mesh = AnimPlayer->GetMesh();
    if (!Mesh) return;

    UAnimInstance* Anim = Mesh->GetAnimInstance();
    if (Anim)
        Anim->OnMontageEnded.AddDynamic(this, &USwordWeapon::OnMontageEnded);
}

void USwordWeapon::SetupHitbox()
{
    if (UCameraComponent* Cam = GetCameraSafe())
    {
        if (SlashHitbox)
            SlashHitbox->AttachToComponent(Cam, FAttachmentTransformRules::KeepRelativeTransform);
    }
}

void USwordWeapon::OnEquipped_Implementation()
{
    Super::OnEquipped_Implementation();
    PlayDrawAnimation();
}

void USwordWeapon::OnUnequipped_Implementation()
{
    // 무기 해제 시 콤보 강제 리셋
    ACharacter* Owner = GetOwnerSafe();
    if (Owner && Owner->GetWorld())
    {
        Owner->GetWorldTimerManager().ClearTimer(InputWindowTimer);
        Owner->GetWorldTimerManager().ClearTimer(ComboResetTimer);
        Owner->GetWorldTimerManager().ClearTimer(ReturnPoseTimer);
    }

    ComboStep        = 0;
    bInputWindowOpen = false;
    bInputBuffered   = false;
    bHitboxActive    = false;
    HitActorsThisSwing.Empty();

    if (SlashHitbox)
        SlashHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Super::OnUnequipped_Implementation();
}

void USwordWeapon::EndPlay(EEndPlayReason::Type Reason)
{
    ACharacter* Owner = GetOwnerSafe();
    if (Owner && Owner->GetWorld())
    {
        Owner->GetWorldTimerManager().ClearTimer(InputWindowTimer);
        Owner->GetWorldTimerManager().ClearTimer(ComboResetTimer);
        Owner->GetWorldTimerManager().ClearTimer(ReturnPoseTimer);
    }
    Super::EndPlay(Reason);
}

bool USwordWeapon::TryAttack_Implementation()
{
    if (!GetOwnerSafe()) return false;

    if (bInputWindowOpen)
    {
        bInputBuffered = true;
        return true;
    }

    if (ComboStep == 0)
    {
        PerformAttack();
        return true;
    }

    return false;
}

void USwordWeapon::PerformAttack()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    Owner->GetWorldTimerManager().ClearTimer(InputWindowTimer);
    Owner->GetWorldTimerManager().ClearTimer(ComboResetTimer);
    Owner->GetWorldTimerManager().ClearTimer(ReturnPoseTimer);

    bInputWindowOpen = false;
    bInputBuffered   = false;

    ComboStep = FMath::Clamp(ComboStep + 1, 1, ComboDamages.Num());

    LastDirection = (LastDirection == ESlashDirection::RightToLeft)
        ? ESlashDirection::LeftToRight
        : ESlashDirection::RightToLeft;

    HitActorsThisSwing.Empty();

    PlayComboMontage();
    EnableHitbox();

    OnSlash.Broadcast(LastDirection);
    OnComboChanged.Broadcast(ComboStep);
}

void USwordWeapon::PlayComboMontage()
{
    if (!SwordMontage) return;

    const int32 Index   = FMath::Clamp(ComboStep - 1, 0, ComboSections.Num() - 1);
    const FName Section = ComboSections.IsValidIndex(Index) ? ComboSections[Index] : NAME_None;
    if (Section == NAME_None) return;

    PlayMontageSection(SwordMontage, Section);
}

void USwordWeapon::PlayDrawAnimation()
{
    if (!DrawMontage) return;
    PlayMontage(DrawMontage);
}

void USwordWeapon::EnableHitbox()
{
    if (!SlashHitbox) return;

    const int32 Index = FMath::Clamp(ComboStep - 1, 0, ComboHitboxDurations.Num() - 1);
    HitboxTimer   = ComboHitboxDurations.IsValidIndex(Index) ? ComboHitboxDurations[Index] : 0.2f;
    bHitboxActive = true;
    SlashHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void USwordWeapon::DisableHitbox()
{
    if (!SlashHitbox) return;
    bHitboxActive = false;
    SlashHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OpenInputWindow();
}

void USwordWeapon::OpenInputWindow()
{
    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    bInputWindowOpen = true;
    bInputBuffered   = false;

    Owner->GetWorldTimerManager().SetTimer(InputWindowTimer, [this]()
    {
        CloseInputWindow();
    }, InputWindowDuration, false);
}

void USwordWeapon::CloseInputWindow()
{
    bInputWindowOpen = false;

    if (bInputBuffered && ComboStep < ComboDamages.Num())
    {
        bInputBuffered = false;
        PerformAttack();
    }
    else
    {
        ACharacter* Owner = GetOwnerSafe();
        if (!Owner || !Owner->GetWorld()) return;

        Owner->GetWorldTimerManager().SetTimer(ComboResetTimer, [this]()
        {
            ResetCombo();
        }, ComboResetDelay, false);
    }
}

void USwordWeapon::ResetCombo()
{
    ComboStep        = 0;
    bInputWindowOpen = false;
    bInputBuffered   = false;
    HitActorsThisSwing.Empty();
    OnComboChanged.Broadcast(0);

    // 콤보 리셋 시 Draw 애니메이션 재생
    PlayDrawAnimation();
}

void USwordWeapon::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != SwordMontage) return;
    if (bInterrupted) return;

    UAnimationPlayerComponent* AnimPlayer = Cast<UAnimationPlayerComponent>(
        AnimationPlayer.GetObject());
    if (!AnimPlayer) return;

    USkeletalMeshComponent* Mesh = AnimPlayer->GetMesh();
    if (!Mesh) return;

    UAnimInstance* Anim = Mesh->GetAnimInstance();
    if (!Anim) return;

    Anim->Montage_Pause(SwordMontage);

    ACharacter* Owner = GetOwnerSafe();
    if (!Owner || !Owner->GetWorld()) return;

    Owner->GetWorldTimerManager().SetTimer(ReturnPoseTimer, [this]()
    {
        UAnimationPlayerComponent* AP = Cast<UAnimationPlayerComponent>(
            AnimationPlayer.GetObject());
        if (!AP) return;

        USkeletalMeshComponent* M = AP->GetMesh();
        if (!M) return;

        UAnimInstance* A = M->GetAnimInstance();
        if (A) A->Montage_Resume(SwordMontage);

    }, ReturnPoseDelay, false);
}

void USwordWeapon::OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                   bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Owner = GetOwnerSafe();
    if (!OtherActor || OtherActor == Owner) return;
    if (!OtherActor->Implements<UDamageable>()) return;
    if (HitActorsThisSwing.Contains(OtherActor)) return;

    HitActorsThisSwing.Add(OtherActor);

    const int32 Index  = FMath::Clamp(ComboStep - 1, 0, ComboDamages.Num() - 1);
    const float Damage = ComboDamages.IsValidIndex(Index) ? ComboDamages[Index] : 80.f;

    FHitResultBuilder Builder;
    Builder
        .From(Owner ? Owner->GetController() : nullptr)
        .Target(OtherActor)
        .At(SweepResult.ImpactPoint, SweepResult.ImpactNormal)
        .WithDamage(Damage)
        .OfDamageType(EWeaponDamageType::Sword)
        .OfHitType(EHitType::Normal);

    OnSwordHit.Broadcast(Builder.Build());
    Builder.Apply();
}

void USwordWeapon::TickComponent(float DeltaTime, ELevelTick TickType,
                                 FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bHitboxActive) return;
    HitboxTimer -= DeltaTime;
    if (HitboxTimer <= 0.f)
        DisableHitbox();
}