#include "Weapons/Registry/WeaponRegistry.h"
#include "Weapons/Interfaces/WeaponSkill.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UWeaponRegistry::Register(TScriptInterface<IWeapon> Weapon)
{
    if (!Weapon.GetObject()) return;

    const FName Id = IWeapon::Execute_GetWeaponId(Weapon.GetObject());
    if (Id.IsNone()) return;

    if (IndexForId(Id) != INDEX_NONE) return;

    Weapons.Add(Weapon);

    if (CurrentIndex == INDEX_NONE)
    {
        CurrentIndex = Weapons.Num() - 1;
        IWeapon::Execute_OnEquipped(Weapon.GetObject());

        FWeaponChangedEvent Event;
        Event.PreviousWeaponId = NAME_None;
        Event.NewWeaponId      = Id;
        OnWeaponChanged.Broadcast(Event);
    }
}

bool UWeaponRegistry::SwapTo(FName WeaponId)
{
    if (bIsSwapping)              return false;
    if (IsCurrentSkillActive())   return false;
    if (WeaponId.IsNone())        return false;

    const int32 NewIndex = IndexForId(WeaponId);
    if (NewIndex == INDEX_NONE)   return false;
    if (NewIndex == CurrentIndex) return true;

    PerformSwap(NewIndex);
    return true;
}

void UWeaponRegistry::CycleNext()
{
    if (Weapons.Num() <= 1)     return;
    if (bIsSwapping)            return;
    if (IsCurrentSkillActive()) return;

    PerformSwap((CurrentIndex + 1) % Weapons.Num());
}

void UWeaponRegistry::CyclePrevious()
{
    if (Weapons.Num() <= 1)     return;
    if (bIsSwapping)            return;
    if (IsCurrentSkillActive()) return;

    PerformSwap((CurrentIndex - 1 + Weapons.Num()) % Weapons.Num());
}

TScriptInterface<IWeapon> UWeaponRegistry::GetCurrentWeapon() const
{
    if (Weapons.IsValidIndex(CurrentIndex)) return Weapons[CurrentIndex];
    return nullptr;
}

FName UWeaponRegistry::GetCurrentWeaponId() const
{
    if (!Weapons.IsValidIndex(CurrentIndex)) return NAME_None;
    UObject* Obj = Weapons[CurrentIndex].GetObject();
    return Obj ? IWeapon::Execute_GetWeaponId(Obj) : NAME_None;
}

void UWeaponRegistry::PerformSwap(int32 NewIndex)
{
    const FName PrevId = GetCurrentWeaponId();

    if (Weapons.IsValidIndex(CurrentIndex))
    {
        UObject* PrevObj = Weapons[CurrentIndex].GetObject();
        if (PrevObj) IWeapon::Execute_OnUnequipped(PrevObj);
    }

    CurrentIndex = NewIndex;
    bIsSwapping  = true;

    UObject* NewObj = Weapons.IsValidIndex(CurrentIndex) ? Weapons[CurrentIndex].GetObject() : nullptr;
    if (NewObj) IWeapon::Execute_OnEquipped(NewObj);

    AActor* Owner = GetOwner();
    if (Owner && Owner->GetWorld())
    {
        Owner->GetWorld()->GetTimerManager().SetTimer(
            SwapTimer,
            FTimerDelegate::CreateUObject(this, &UWeaponRegistry::OnSwapComplete),
            SwapDuration > 0.f ? SwapDuration : 0.001f,
            false);
    }
    else
    {
        bIsSwapping = false;
    }

    FWeaponChangedEvent Event;
    Event.PreviousWeaponId = PrevId;
    Event.NewWeaponId      = GetCurrentWeaponId();
    OnWeaponChanged.Broadcast(Event);
}

void UWeaponRegistry::OnSwapComplete()
{
    bIsSwapping = false;
}

bool UWeaponRegistry::IsCurrentSkillActive() const
{
    if (!Weapons.IsValidIndex(CurrentIndex)) return false;

    UObject* Obj = Weapons[CurrentIndex].GetObject();
    if (!Obj) return false;

    TScriptInterface<IWeaponSkill> Skill = IWeapon::Execute_GetSkill(Obj);
    UObject* SkillObj = Skill.GetObject();
    return SkillObj && IWeaponSkill::Execute_IsActive(SkillObj);
}

int32 UWeaponRegistry::IndexForId(FName WeaponId) const
{
    if (WeaponId.IsNone()) return INDEX_NONE;

    for (int32 i = 0; i < Weapons.Num(); ++i)
    {
        UObject* Obj = Weapons[i].GetObject();
        if (!Obj) continue;
        if (IWeapon::Execute_GetWeaponId(Obj) == WeaponId)
            return i;
    }
    return INDEX_NONE;
}