#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponSkill.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UWeaponSkill : public UInterface
{
	GENERATED_BODY()
};

class FPS_API IWeaponSkill
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Skill")
	void StartHold();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Skill")
	void EndHold();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Skill")
	bool IsSkillActive() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Skill")
	bool BlocksMovement() const;
};