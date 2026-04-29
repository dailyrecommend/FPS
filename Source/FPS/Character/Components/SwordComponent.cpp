#include "SwordComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Combat/DamageableInterface.h"

USwordComponent::USwordComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    SlashHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("SlashHitbox"));
    SlashHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SlashHitbox->SetCollisionObjectType(ECC_WorldDynamic);
    SlashHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
    SlashHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SlashHitbox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void USwordComponent::BeginPlay()
{
    Super::BeginPlay();
    SlashHitbox->OnComponentBeginOverlap.AddDynamic(this, &USwordComponent::OnHitboxOverlap);
}

void USwordComponent::Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera)
{
    OwnerCharacter = InOwner;
    Camera         = InCamera;

    // Attach to camera so hitbox always stays in front of player view
    if (Camera)
    {
        SlashHitbox->AttachToComponent(
            Camera,
            FAttachmentTransformRules::KeepRelativeTransform
        );
    }
}

bool USwordComponent::CanAttack() const
{
    if (!OwnerCharacter) return false;
    return (OwnerCharacter->GetWorld()->GetTimeSeconds() - LastAttackTime) >= AttackCooldown;
}

void USwordComponent::TryAttack()
{
    if (!CanAttack()) return;
    PerformAttack();
}

void USwordComponent::PerformAttack()
{
    LastAttackTime = OwnerCharacter->GetWorld()->GetTimeSeconds();

    // Alternate slash direction
    LastDirection = (LastDirection == ESlashDirection::RightToLeft)
        ? ESlashDirection::LeftToRight
        : ESlashDirection::RightToLeft;

    PlayMontage(LastDirection == ESlashDirection::RightToLeft
        ? SlashRightToLeftMontage
        : SlashLeftToRightMontage);

    EnableHitbox();
    OnSlash.Broadcast(LastDirection);
}

void USwordComponent::EnableHitbox()
{
    bHitboxActive     = true;
    HitboxActiveTimer = HitboxActiveDuration;
    SlashHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void USwordComponent::DisableHitbox()
{
    bHitboxActive = false;
    SlashHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void USwordComponent::OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                       bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == OwnerCharacter) return;
    BroadcastHit(OtherActor, SweepResult.ImpactPoint, SweepResult.ImpactNormal);
}

void USwordComponent::BroadcastHit(AActor* HitActor, const FVector& Location, const FVector& Normal)
{
    FWeaponHitResult WeaponHit;
    WeaponHit.HitActor    = HitActor;
    WeaponHit.HitLocation = Location;
    WeaponHit.HitNormal   = Normal;
    WeaponHit.Damage      = AttackDamage;
    WeaponHit.DamageType  = EWeaponDamageType::Sword;
    WeaponHit.HitType     = EHitType::Normal;
    WeaponHit.bIsCritical = false;
    WeaponHit.Instigator  = OwnerCharacter->GetController();

    OnHit.Broadcast(WeaponHit);

    if (HitActor->Implements<UDamageable>())
        IDamageable::Execute_OnWeaponHit(HitActor, WeaponHit);
}

void USwordComponent::PlayMontage(UAnimMontage* Montage)
{
    if (!Montage || !OwnerCharacter) return;

    UAnimInstance* Anim = OwnerCharacter->GetArmsMesh()->GetAnimInstance();
    if (Anim) Anim->Montage_Play(Montage, 1.f);
}

void USwordComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bHitboxActive) return;
    HitboxActiveTimer -= DeltaTime;
    if (HitboxActiveTimer <= 0.f) DisableHitbox();
}