#include "Weapons/Implementations/Sword/SwordWeapon.h"
#include "Combat/Builders/HitResultBuilder.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"

USwordWeapon::USwordWeapon()
{
    WeaponId = TEXT("Sword");
    Cooldown = 0.4f;

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

    if (UCameraComponent* Cam = GetCameraSafe(); Cam && SlashHitbox)
    {
        SlashHitbox->AttachToComponent(
            Cam,
            FAttachmentTransformRules::KeepRelativeTransform);
    }
}

bool USwordWeapon::TryAttack_Implementation()
{
    if (!IsCooldownReady()) return false;
    if (!GetOwnerSafe())    return false;

    LastDirection = (LastDirection == ESlashDirection::RightToLeft)
        ? ESlashDirection::LeftToRight
        : ESlashDirection::RightToLeft;

    PlayMontage(LastDirection == ESlashDirection::RightToLeft
        ? SlashRightToLeftMontage
        : SlashLeftToRightMontage);

    EnableHitbox();
    OnSlash.Broadcast(LastDirection);
    StartCooldown();
    return true;
}

void USwordWeapon::EnableHitbox()
{
    if (!SlashHitbox) return;
    bHitboxActive     = true;
    HitboxActiveTimer = HitboxActiveDuration;
    SlashHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void USwordWeapon::DisableHitbox()
{
    if (!SlashHitbox) return;
    bHitboxActive = false;
    SlashHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void USwordWeapon::OnHitboxOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
                                   UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
                                   bool /*bFromSweep*/, const FHitResult& SweepResult)
{
    ACharacter* Owner = GetOwnerSafe();
    if (!OtherActor || OtherActor == Owner) return;

    FHitResultBuilder Builder;
    Builder
        .From(Owner ? Owner->GetController() : nullptr)
        .Target(OtherActor)
        .At(SweepResult.ImpactPoint, SweepResult.ImpactNormal)
        .WithDamage(AttackDamage)
        .OfDamageType(EWeaponDamageType::Sword)
        .OfHitType(EHitType::Normal);

    OnSwordHit.Broadcast(Builder.Build());
    Builder.Apply();
}

void USwordWeapon::PlayMontage(UAnimMontage* Montage)
{
    if (!Montage) return;

    ACharacter* Owner = GetOwnerSafe();
    if (!Owner) return;

    USkeletalMeshComponent* Mesh = Owner->GetMesh();
    if (!Mesh) return;

    UAnimInstance* Anim = Mesh->GetAnimInstance();
    if (Anim) Anim->Montage_Play(Montage, 1.f);
}

void USwordWeapon::TickComponent(float DeltaTime, ELevelTick TickType,
                                 FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bHitboxActive) return;
    HitboxActiveTimer -= DeltaTime;
    if (HitboxActiveTimer <= 0.f) DisableHitbox();
}