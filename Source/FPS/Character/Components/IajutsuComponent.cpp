#include "IajutsuComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "SwordComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Combat/DamageableInterface.h"

UIajutsuComponent::UIajutsuComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UIajutsuComponent::Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera, USwordComponent* InSword)
{
    OwnerCharacter = InOwner;
    Camera         = InCamera;
    Sword          = InSword;

    if (Sword && Sword->SlashHitbox)
    {
        Sword->SlashHitbox->OnComponentBeginOverlap.AddDynamic(
            this, &UIajutsuComponent::OnHitboxOverlapDuringIajutsu
        );
    }
}

bool UIajutsuComponent::CanIajutsu() const
{
    return !bIsIajutsu && CooldownRemaining <= 0.f;
}

void UIajutsuComponent::StartIajutsu()
{
    if (!CanIajutsu() || !OwnerCharacter) return;
    PerformIajutsu();
}

void UIajutsuComponent::PerformIajutsu()
{
    bIsIajutsu     = true;
    IajutsuElapsed = 0.f;
    HitActors.Empty();

    DashDirection = Camera
        ? Camera->GetForwardVector().GetSafeNormal2D()
        : OwnerCharacter->GetActorForwardVector();

    OwnerCharacter->GetCharacterMovement()->GravityScale = 0.f;
    OwnerCharacter->GetCharacterMovement()->Velocity     = DashDirection * IajutsuSpeed;

    if (Sword) Sword->SlashHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    PlayMontage(IajutsuMontage);
    OnIajutsuStarted.Broadcast();
}

void UIajutsuComponent::EndIajutsu()
{
    bIsIajutsu        = false;
    CooldownRemaining = IajutsuCooldown;
    HitActors.Empty();

    if (Sword) Sword->SlashHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OwnerCharacter->GetCharacterMovement()->GravityScale = 1.f;
    OwnerCharacter->GetCharacterMovement()->Velocity     = DashDirection * IajutsuExitMomentum;

    OnIajutsuEnded.Broadcast();
}

void UIajutsuComponent::OnHitboxOverlapDuringIajutsu(UPrimitiveComponent* OverlappedComp,
                                                      AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp,
                                                      int32 OtherBodyIndex,
                                                      bool bFromSweep,
                                                      const FHitResult& SweepResult)
{
    if (!bIsIajutsu) return;
    if (!OtherActor || OtherActor == OwnerCharacter) return;
    if (HitActors.Contains(OtherActor)) return;

    HitActors.Add(OtherActor);
    BroadcastHit(OtherActor, SweepResult.ImpactPoint, SweepResult.ImpactNormal);
}

void UIajutsuComponent::BroadcastHit(AActor* HitActor, const FVector& Location, const FVector& Normal)
{
    FWeaponHitResult WeaponHit;
    WeaponHit.HitActor    = HitActor;
    WeaponHit.HitLocation = Location;
    WeaponHit.HitNormal   = Normal;
    WeaponHit.Damage      = IajutsuDamage;
    WeaponHit.DamageType  = EWeaponDamageType::Sword;
    WeaponHit.HitType     = EHitType::Iajutsu;
    WeaponHit.bIsCritical = false;
    WeaponHit.Instigator  = OwnerCharacter->GetController();

    OnHit.Broadcast(WeaponHit);

    if (HitActor->Implements<UDamageable>())
        IDamageable::Execute_OnWeaponHit(HitActor, WeaponHit);
}

void UIajutsuComponent::TickDash(float DeltaTime)
{
    if (!bIsIajutsu || !OwnerCharacter) return;

    IajutsuElapsed += DeltaTime;

    if (IajutsuElapsed * IajutsuSpeed >= IajutsuDistance)
    {
        EndIajutsu();
        return;
    }

    OwnerCharacter->GetCharacterMovement()->Velocity = DashDirection * IajutsuSpeed;
}

void UIajutsuComponent::TickCooldown(float DeltaTime)
{
    if (CooldownRemaining > 0.f)
        CooldownRemaining = FMath::Max(0.f, CooldownRemaining - DeltaTime);
}

void UIajutsuComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TickDash(DeltaTime);
    TickCooldown(DeltaTime);
}

void UIajutsuComponent::PlayMontage(UAnimMontage* Montage)
{
    if (!Montage || !OwnerCharacter) return;

    UAnimInstance* Anim = OwnerCharacter->GetArmsMesh()->GetAnimInstance();
    if (Anim) Anim->Montage_Play(Montage, 1.f);
}