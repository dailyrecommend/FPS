#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponSwapComponent.generated.h"

class APlayerCharacter;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Gun,
    Sword,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwapped, EWeaponType, NewWeapon);

/**
 * Manages weapon switching between Gun and Sword.
 * Holds direct mesh references instead of searching by name.
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UWeaponSwapComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeaponSwapComponent();

    /**
     * @param InGunMesh   Direct reference to the gun skeletal mesh.
     * @param InSwordMesh Direct reference to the sword skeletal mesh.
     */
    void Initialize(APlayerCharacter* InOwner, USkeletalMeshComponent* InGunMesh, USkeletalMeshComponent* InSwordMesh);

    void SwapToGun();
    void SwapToSword();
    void SwapScroll(float ScrollValue);

    UPROPERTY(BlueprintAssignable)
    FOnWeaponSwapped OnWeaponSwapped;

    UFUNCTION(BlueprintPure) EWeaponType GetCurrentWeapon() const { return CurrentWeapon; }
    UFUNCTION(BlueprintPure) bool        IsSwapping()       const { return bIsSwapping; }

private:
    void PerformSwap(EWeaponType NewWeapon);
    void UpdateMeshVisibility(EWeaponType NewWeapon) const;
    void PlayDrawMontage(EWeaponType NewWeapon) const;
    void OnSwapComplete();

    UPROPERTY(EditDefaultsOnly, Category = "WeaponSwap")
    float SwapDuration = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "WeaponSwap")
    UAnimMontage* GunDrawMontage;

    UPROPERTY(EditDefaultsOnly, Category = "WeaponSwap")
    UAnimMontage* SwordDrawMontage;

    UPROPERTY() APlayerCharacter*        OwnerCharacter = nullptr;
    UPROPERTY() USkeletalMeshComponent*  GunMesh        = nullptr;
    UPROPERTY() USkeletalMeshComponent*  SwordMesh      = nullptr;

    EWeaponType  CurrentWeapon = EWeaponType::Gun;
    bool         bIsSwapping   = false;
    FTimerHandle SwapTimer;
};