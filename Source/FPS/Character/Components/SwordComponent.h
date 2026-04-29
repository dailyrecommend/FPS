#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwordComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwordAttack);

/**
 * 검 기본 공격만 담당.
 * 발도술 스킬은 IajutsuComponent가 처리.
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USwordComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USwordComponent();

    void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera);

    void TryAttack();

    UPROPERTY(BlueprintAssignable)
    FOnSwordAttack OnSwordAttack;

    UFUNCTION(BlueprintPure) bool CanAttack() const;

    /**
     * BP에서 배치할 히트박스.
     * 크기와 위치는 BP Details 패널에서 조정.
     * 코드에서는 공격 타이밍에만 Overlap을 켜고 끔.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sword|Hitbox")
    UBoxComponent* SlashHitbox;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void PerformAttack();
    void EnableHitbox();
    void DisableHitbox();
    void PlayMontage(UAnimMontage* Montage);

    UFUNCTION()
    void OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                         bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack")
    float AttackDamage = 80.f;

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack")
    float AttackCooldown = 0.4f;

    /** 히트박스가 활성화되는 시간 (스윙 구간) */
    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack")
    float HitboxActiveDuration = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack")
    UAnimMontage* SlashMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack")
    UAnimMontage* StabMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Hitbox")
    FName HitboxSocketName = TEXT("Socket_Sword");

    UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
    UPROPERTY() UCameraComponent* Camera         = nullptr;

    float LastAttackTime    = 0.f;
    float HitboxActiveTimer = 0.f;
    bool  bHitboxActive     = false;
    bool  bSlashTurn        = true;   // true = 베기, false = 찌르기 교대
};