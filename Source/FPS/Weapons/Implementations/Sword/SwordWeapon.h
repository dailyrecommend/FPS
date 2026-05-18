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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboChanged, int32, ComboStep);

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USwordWeapon : public UWeaponBase
{
    GENERATED_BODY()

public:
    USwordWeapon();

    UPROPERTY(BlueprintAssignable, Category = "Sword") FOnSlash        OnSlash;
    UPROPERTY(BlueprintAssignable, Category = "Sword") FOnSwordHit     OnSwordHit;
    UPROPERTY(BlueprintAssignable, Category = "Sword") FOnComboChanged OnComboChanged;

    UFUNCTION(BlueprintPure, Category = "Sword") ESlashDirection GetLastDirection() const { return LastDirection; }
    UFUNCTION(BlueprintPure, Category = "Sword") int32           GetComboStep()     const { return ComboStep; }
    UFUNCTION(BlueprintPure, Category = "Sword") bool            IsInCombo()        const { return ComboStep > 0; }

    void SetupHitbox();

    virtual void AttachAnimationPlayer(TScriptInterface<IAnimationPlayer> InPlayer);

    virtual void OnEquipped_Implementation()   override;
    virtual void OnUnequipped_Implementation() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sword|Hitbox")
    TObjectPtr<UBoxComponent> SlashHitbox;

    virtual bool TryAttack_Implementation() override;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(EEndPlayReason::Type Reason) override;

private:
    void PerformAttack();
    void EnableHitbox();
    void DisableHitbox();
    void ResetCombo();
    void OpenInputWindow();
    void CloseInputWindow();
    void PlayComboMontage();
    void PlayDrawAnimation();

    UFUNCTION()
    void OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                         bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Combo")
    TArray<float> ComboDamages = { 80.f, 90.f, 120.f };

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Combo")
    TArray<float> ComboHitboxDurations = { 0.2f, 0.2f, 0.25f };

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Combo")
    float InputWindowDuration = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Combo")
    float ComboResetDelay = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Anim")
    TObjectPtr<UAnimMontage> SwordMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Anim")
    TArray<FName> ComboSections = { TEXT("Slash1"), TEXT("Slash2"), TEXT("Slash3") };

    UPROPERTY(EditDefaultsOnly, Category = "Sword|Anim")
    float ReturnPoseDelay = 0.1f;

    int32 ComboStep        = 0;
    bool  bInputWindowOpen = false;
    bool  bInputBuffered   = false;

    float HitboxTimer   = 0.f;
    bool  bHitboxActive = false;

    ESlashDirection LastDirection = ESlashDirection::RightToLeft;

    TSet<AActor*> HitActorsThisSwing;

    FTimerHandle InputWindowTimer;
    FTimerHandle ComboResetTimer;
    FTimerHandle ReturnPoseTimer;
};