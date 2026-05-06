#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Movement/Enums/EActivationResult.h"
#include "Movement/Data/AbilityContext.h"
#include "Ability.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UAbility : public UInterface
{
	GENERATED_BODY()
};

class FPS_API IAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement|Ability")
	EActivationResult TryActivate(const FAbilityContext& Context);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement|Ability")
	void Deactivate();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement|Ability")
	bool IsAbilityActive() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement|Ability")
	FName GetAbilityId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement|Ability")
	bool CanBeInterruptedBy(FName OtherAbilityId) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement|Ability")
	bool RequestCancel();
};