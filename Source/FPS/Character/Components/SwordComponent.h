#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/WeaponHitResult.h"
#include "SwordComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class ESlashDirection : uint8
{
    RightToLeft,  // 우상단 -> 좌하단
    LeftToRight,  // 좌상단 -> 우하단
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlash, ESlashDirection, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwordHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USwordComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USwordComponent();

    void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera);

    void TryAttack();

    // Broadcasts slash direction for animation
    UPROPERTY(BlueprintAssignable) FOnSlash   OnSlash;
    UPROPERTY(BlueprintAssignable) FOnSwordHit OnHit;

    UFUNCTION(BlueprintPure) bool             CanAttack()        const;
    UFUNCTION(BlueprintPure) ESlashDirection  GetLastDirection() const { return LastDirection; }

    // Size and offset managed in Blueprint
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
    void BroadcastHit(AActor* HitActor, const FVector& Location, const FVector& Normal);
    void PlayMontage(UAnimMontage* Montage);

    UFUNCTION()
    void OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                         bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack") float         AttackDamage         = 80.f;
    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack") float         AttackCooldown       = 0.4f;
    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack") float         HitboxActiveDuration = 0.2f;
    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack") UAnimMontage* SlashRightToLeftMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Sword|Attack") UAnimMontage* SlashLeftToRightMontage;

    UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;
    UPROPERTY() UCameraComponent* Camera         = nullptr;

    float          LastAttackTime    = 0.f;
    float          HitboxActiveTimer = 0.f;
    bool           bHitboxActive     = false;
    ESlashDirection LastDirection    = ESlashDirection::RightToLeft;
};