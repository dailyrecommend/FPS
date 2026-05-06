#pragma once
#include "CoreMinimal.h"
#include "Weapons/Base/WeaponBase.h"
#include "Weapons/Implementations/Sword/Enums/ESlashDirection.h"
#include "Combat/Data/WeaponHitResult.h"
#include "SwordWeapon.generated.h"

class UAnimMontage;
class UBoxComponent;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlash, ESlashDirection, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwordHit, const FWeaponHitResult&, HitResult);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USwordWeapon : public UWeaponBase
{
    GENERATED_BODY()

public:
    USwordWeapon();

    /**
     * Wires the slash hitbox to the camera and binds the overlap delegate.
     * Must be called after InjectDependencies(...) so GetCameraSafe() is valid.
     * Idempotent — safe to call multiple times.
     */
    void SetupHitbox();

    UPROPERTY(BlueprintAssignable, Category = "Sword") FOnSlash    OnSlash;
    UPROPERTY(BlueprintAssignable, Category = "Sword") FOnSwordHit OnSwordHit;

    UFUNCTION(BlueprintPure, Category = "Sword") ESlashDirection GetLastDirection() const { return LastDirection; }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sword|Hitbox")
    TObjectPtr<UBoxComponent> SlashHitbox;

    virtual bool TryAttack_Implementation() override;

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void EnableHitbox();
    void DisableHitbox();

    UFUNCTION()
    void OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                         bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(EditDefaultsOnly, Category = "Sword") float                   AttackDamage         = 80.f;
    UPROPERTY(EditDefaultsOnly, Category = "Sword") float                   HitboxActiveDuration = 0.2f;
    UPROPERTY(EditDefaultsOnly, Category = "Sword") TObjectPtr<UAnimMontage> SlashRightToLeftMontage;
    UPROPERTY(EditDefaultsOnly, Category = "Sword") TObjectPtr<UAnimMontage> SlashLeftToRightMontage;

    float           HitboxActiveTimer = 0.f;
    bool            bHitboxActive     = false;
    ESlashDirection LastDirection     = ESlashDirection::RightToLeft;

    bool bHitboxSetupDone = false;
};