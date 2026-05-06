#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Movement/Interfaces/Ability.h"
#include "AbilityRegistry.generated.h"

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UAbilityRegistry : public UActorComponent
{
	GENERATED_BODY()

public:
	void Register(TScriptInterface<IAbility> Ability);

	UFUNCTION(BlueprintPure)
	TScriptInterface<IAbility> Find(FName AbilityId) const;

	UFUNCTION(BlueprintPure)
	bool IsAnyAbilityActive() const;

	UFUNCTION(BlueprintPure)
	bool IsAbilityActive(FName AbilityId) const;

	UFUNCTION(BlueprintCallable)
	EActivationResult TryActivate(FName AbilityId, const FAbilityContext& Context);

	UFUNCTION(BlueprintCallable)
	bool Cancel(FName AbilityId);

	UFUNCTION(BlueprintCallable)
	void CancelInterruptibleBy(FName InitiatorAbilityId);

private:
	UPROPERTY()
	TArray<TScriptInterface<IAbility>> Abilities;
};