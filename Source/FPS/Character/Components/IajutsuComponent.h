#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IajutsuComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;
class USwordComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIajutsuStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIajutsuEnded);

/**
 * 발도술 스킬 담당.
 * 전방으로 돌진하면서 경로상 모든 적을 SwordComponent의 SlashHitbox로 처치.
 * FocusComponent와 동일한 구조 — 기본 공격(SwordComponent)과 완전히 분리.
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UIajutsuComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UIajutsuComponent();

    void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera, USwordComponent* InSword);

    void StartIajutsu();

    UPROPERTY(BlueprintAssignable) FOnIajutsuStarted OnIajutsuStarted;
    UPROPERTY(BlueprintAssignable) FOnIajutsuEnded   OnIajutsuEnded;

    UFUNCTION(BlueprintPure) bool  IsIajutsu()            const { return bIsIajutsu; }
    UFUNCTION(BlueprintPure) bool  CanIajutsu()           const;
    UFUNCTION(BlueprintPure) float GetCooldownRemaining() const { return CooldownRemaining; }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void PerformIajutsu();
    void EndIajutsu();
    void TickDash(float DeltaTime);
    void TickCooldown(float DeltaTime);
    void PlayMontage(UAnimMontage* Montage);

    UFUNCTION()
    void OnHitboxOverlapDuringIajutsu(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                      bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu")
    float IajutsuDamage = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu")
    float IajutsuSpeed = 3000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu")
    float IajutsuDistance = 1500.f;

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu")
    float IajutsuCooldown = 6.f;

    /** 돌진 종료 후 남는 수평 관성 속도 */
    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu")
    float IajutsuExitMomentum = 800.f;

    UPROPERTY(EditDefaultsOnly, Category = "Iajutsu")
    UAnimMontage* IajutsuMontage;

    UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
    UPROPERTY() UCameraComponent* Camera         = nullptr;
    UPROPERTY() USwordComponent*  Sword          = nullptr;

    bool    bIsIajutsu        = false;
    float   IajutsuElapsed    = 0.f;
    float   CooldownRemaining = 0.f;
    FVector DashDirection     = FVector::ZeroVector;

    // 돌진 중 이미 맞은 액터 (중복 데미지 방지)
    TSet<AActor*> HitActors;
};