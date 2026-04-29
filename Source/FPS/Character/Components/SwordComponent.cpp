#include "SwordComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

USwordComponent::USwordComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 히트박스 생성 — 크기/위치는 BP에서 관리
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

    if (OwnerCharacter && OwnerCharacter->GetArmsMesh())
    {
        SlashHitbox->AttachToComponent(
            OwnerCharacter->GetArmsMesh(),
            FAttachmentTransformRules::KeepRelativeTransform,
            HitboxSocketName
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

    PlayMontage(bSlashTurn ? SlashMontage : StabMontage);
    bSlashTurn = !bSlashTurn;

    EnableHitbox();
    OnSwordAttack.Broadcast();
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

    UGameplayStatics::ApplyDamage(
        OtherActor,
        AttackDamage,
        OwnerCharacter->GetController(),
        OwnerCharacter,
        UDamageType::StaticClass()
    );
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
    if (HitboxActiveTimer <= 0.f)
        DisableHitbox();
}