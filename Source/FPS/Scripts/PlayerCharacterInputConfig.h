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
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* IMC_KeyboardMouse;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* IMC_Gamepad;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_Jump;
};