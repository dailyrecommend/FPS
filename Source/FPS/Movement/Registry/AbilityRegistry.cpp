#include "Movement/Registry/AbilityRegistry.h"

void UAbilityRegistry::Register(TScriptInterface<IAbility> Ability)
{
    if (!Ability.GetObject()) return;

    const FName Id = IAbility::Execute_GetAbilityId(Ability.GetObject());
    if (Id.IsNone()) return;

    if (Find(Id).GetObject() != nullptr) return;

    Abilities.Add(Ability);
}

TScriptInterface<IAbility> UAbilityRegistry::Find(FName AbilityId) const
{
    if (AbilityId.IsNone()) return nullptr;

    for (const TScriptInterface<IAbility>& Ability : Abilities)
    {
        UObject* Obj = Ability.GetObject();
        if (!Obj) continue;
        if (IAbility::Execute_GetAbilityId(Obj) == AbilityId)
            return Ability;
    }
    return nullptr;
}

bool UAbilityRegistry::IsAnyActive() const
{
    for (const TScriptInterface<IAbility>& Ability : Abilities)
    {
        UObject* Obj = Ability.GetObject();
        if (Obj && IAbility::Execute_IsActive(Obj)) return true;
    }
    return false;
}

bool UAbilityRegistry::IsActive(FName AbilityId) const
{
    TScriptInterface<IAbility> Ability = Find(AbilityId);
    return Ability.GetObject() && IAbility::Execute_IsActive(Ability.GetObject());
}

EActivationResult UAbilityRegistry::TryActivate(FName AbilityId, const FAbilityContext& Context)
{
    TScriptInterface<IAbility> Ability = Find(AbilityId);
    if (!Ability.GetObject())
        return EActivationResult::Failed_NotReady;

    return IAbility::Execute_TryActivate(Ability.GetObject(), Context);
}

bool UAbilityRegistry::Cancel(FName AbilityId)
{
    TScriptInterface<IAbility> Ability = Find(AbilityId);
    if (!Ability.GetObject()) return false;
    return IAbility::Execute_RequestCancel(Ability.GetObject());
}

void UAbilityRegistry::CancelInterruptibleBy(FName InitiatorAbilityId)
{
    if (InitiatorAbilityId.IsNone()) return;

    for (const TScriptInterface<IAbility>& Ability : Abilities)
    {
        UObject* Obj = Ability.GetObject();
        if (!Obj) continue;
        if (!IAbility::Execute_IsActive(Obj)) continue;
        if (IAbility::Execute_CanBeInterruptedBy(Obj, InitiatorAbilityId))
            IAbility::Execute_RequestCancel(Obj);
    }
}