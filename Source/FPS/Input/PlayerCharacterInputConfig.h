#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCharacterInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class FPS_API UPlayerCharacterInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Input|Mapping") TObjectPtr<UInputMappingContext> IMC_KeyboardMouse;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Mapping") TObjectPtr<UInputMappingContext> IMC_Gamepad;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement") TObjectPtr<UInputAction> IA_Move;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement") TObjectPtr<UInputAction> IA_Look;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement") TObjectPtr<UInputAction> IA_Jump;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement") TObjectPtr<UInputAction> IA_Slide;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement") TObjectPtr<UInputAction> IA_Dash;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement") TObjectPtr<UInputAction> IA_Slam;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat") TObjectPtr<UInputAction> IA_Attack;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat") TObjectPtr<UInputAction> IA_WeaponSkill;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Weapon") TObjectPtr<UInputAction> IA_WeaponSwapGun;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Weapon") TObjectPtr<UInputAction> IA_WeaponSwapSword;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Weapon") TObjectPtr<UInputAction> IA_WeaponSwapScroll;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Weapon") TObjectPtr<UInputAction> IA_WeaponSwapNext;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Weapon") TObjectPtr<UInputAction> IA_WeaponSwapPrev;
};