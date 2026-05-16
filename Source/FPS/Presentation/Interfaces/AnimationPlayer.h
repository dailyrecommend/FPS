#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AnimationPlayer.generated.h"

class UAnimMontage;

UINTERFACE(MinimalAPI, BlueprintType)
class UAnimationPlayer : public UInterface
{
	GENERATED_BODY()
};

class FPS_API IAnimationPlayer
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	void PlayMontage(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	void PlayMontageSection(UAnimMontage* Montage, FName SectionName, float PlayRate);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	void StopMontage(UAnimMontage* Montage, float BlendOutTime);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	bool IsMontagePlaying(UAnimMontage* Montage) const;
};