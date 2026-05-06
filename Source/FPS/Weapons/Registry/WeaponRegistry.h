#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/Interfaces/Weapon.h"
#include "Weapons/Data/WeaponChangedEvent.h"
#include "WeaponRegistry.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChanged, const FWeaponChangedEvent&, Event);

/**
 * Owns the player's set of weapons. Weapons are stored as IWeapon interfaces, not concrete
 * types — adding a weapon requires only registering it, never modifying this class.
 *
 * Swap rules:
 *  - Cannot swap while a hold-skill is active.
 *  - Cannot swap during the swap animation window.
 *  - Bad ids return false instead of crashing.
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UWeaponRegistry : public UActorComponent
{
    GENERATED_BODY()

public:
    /** Register a weapon. Registration order is the cycle order. First registered is auto-equipped. */
    void Register(TScriptInterface<IWeapon> Weapon);

    /** Swap to a weapon by id. Returns false if id is unknown or swap is currently blocked. */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool SwapTo(FName WeaponId);

    UFUNCTION(BlueprintCallable, Category = "Weapon") void CycleNext();
    UFUNCTION(BlueprintCallable, Category = "Weapon") void CyclePrevious();

    UFUNCTION(BlueprintPure, Category = "Weapon")
    TScriptInterface<IWeapon> GetCurrentWeapon() const;

    UFUNCTION(BlueprintPure, Category = "Weapon") FName GetCurrentWeaponId() const;
    UFUNCTION(BlueprintPure, Category = "Weapon") bool  IsSwapping() const { return bIsSwapping; }
    UFUNCTION(BlueprintPure, Category = "Weapon") int32 GetWeaponCount() const { return Weapons.Num(); }

    UPROPERTY(BlueprintAssignable, Category = "Weapon")
    FOnWeaponChanged OnWeaponChanged;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "WeaponSwap")
    float SwapDuration = 0.5f;

private:
    void  PerformSwap(int32 NewIndex);
    void  OnSwapComplete();
    bool  IsCurrentSkillActive() const;
    int32 IndexForId(FName WeaponId) const;

    UPROPERTY()
    TArray<TScriptInterface<IWeapon>> Weapons;

    int32        CurrentIndex = INDEX_NONE;
    bool         bIsSwapping  = false;
    FTimerHandle SwapTimer;
};