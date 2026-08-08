#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AnimationPlayer.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EAnimPriority : uint8
{
    Movement = 0,  // 이동 어빌리티 (슬라이드, 대시 등)
    Attack   = 1,  // 무기 공격
    Skill    = 2,  // 무기 스킬
};

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
	void PlayMontage(UAnimMontage* Montage, float PlayRate, EAnimPriority Priority = EAnimPriority::Movement);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	void PlayMontageSection(UAnimMontage* Montage, FName SectionName, float PlayRate, EAnimPriority Priority = EAnimPriority::Movement);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	void StopMontage(UAnimMontage* Montage, float BlendOutTime);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Presentation")
	bool IsMontagePlaying(UAnimMontage* Montage) const;
};