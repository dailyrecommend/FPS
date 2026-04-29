#include "IajutsuComponent.h"
#include "../PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "SwordComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UIajutsuComponent::UIajutsuComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UIajutsuComponent::Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera, USwordComponent* InSword)
{
    OwnerCharacter = InOwner;
    Camera         = InCamera;
    Sword          = InSword;

    // SwordComponent의 SlashHitbox Overlap 이벤트를 발도술 전용 핸들러로 등록
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

    // 카메라 전방으로 수평 돌진 방향 고정
    DashDirection = Camera
        ? Camera->GetForwardVector().GetSafeNormal2D()
        : OwnerCharacter->GetActorForwardVector();

    // 중력 끄고 수평 돌진
    OwnerCharacter->GetCharacterMovement()->GravityScale = 0.f;
    OwnerCharacter->GetCharacterMovement()->Velocity     = DashDirection * IajutsuSpeed;

    // SwordComponent 히트박스를 발도술 내내 활성화
    if (Sword) Sword->SlashHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    PlayMontage(IajutsuMontage);
    OnIajutsuStarted.Broadcast();
}

void UIajutsuComponent::EndIajutsu()
{
    bIsIajutsu        = false;
    CooldownRemaining = IajutsuCooldown;
    HitActors.Empty();

    // 히트박스 비활성화
    if (Sword) Sword->SlashHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 중력 복구, 수평 관성만 남김
    OwnerCharacter->GetCharacterMovement()->GravityScale = 1.f;
    OwnerCharacter->GetCharacterMovement()->Velocity     = DashDirection * IajutsuExitMomentum;

    OnIajutsuEnded.Broadcast();
}

void UIajutsuComponent::TickDash(float DeltaTime)
{
    if (!bIsIajutsu || !OwnerCharacter) return;

    IajutsuElapsed += DeltaTime;

    const float TraveledDistance = IajutsuElapsed * IajutsuSpeed;
    if (TraveledDistance >= IajutsuDistance)
    {
        EndIajutsu();
        return;
    }

    // 속도 유지
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

void UIajutsuComponent::OnHitboxOverlapDuringIajutsu(UPrimitiveComponent* OverlappedComp,
                                                      AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp,
                                                      int32 OtherBodyIndex,
                                                      bool bFromSweep,
                                                      const FHitResult& SweepResult)
{
    // 발도술 중이 아닐 때는 이 핸들러가 처리하지 않음 (SwordComponent가 처리)
    if (!bIsIajutsu) return;
    if (!OtherActor || OtherActor == OwnerCharacter) return;
    if (HitActors.Contains(OtherActor)) return;

    HitActors.Add(OtherActor);

    UGameplayStatics::ApplyDamage(
        OtherActor,
        IajutsuDamage,
        OwnerCharacter->GetController(),
        OwnerCharacter,
        UDamageType::StaticClass()
    );
}

void UIajutsuComponent::PlayMontage(UAnimMontage* Montage)
{
    if (!Montage || !OwnerCharacter) return;

    UAnimInstance* Anim = OwnerCharacter->GetArmsMesh()->GetAnimInstance();
    if (Anim) Anim->Montage_Play(Montage, 1.f);
}