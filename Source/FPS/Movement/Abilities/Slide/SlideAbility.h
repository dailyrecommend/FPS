#pragma once
#include "CoreMinimal.h"
#include "Movement/Base/AbilityBase.h"
#include "Presentation/Interfaces/CameraEffects.h"
#include "SlideAbility.generated.h"

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USlideAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	USlideAbility();

	void AttachCameraEffects(TScriptInterface<ICameraEffects> InEffects) { CameraEffects = InEffects; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	virtual EActivationResult OnTryActivate(const FAbilityContext& Context) override;
	virtual void              OnDeactivate() override;
	virtual bool              CheckPreconditions(const FAbilityContext& Context) const override;

private:
	void TickSlide(float DeltaTime, const FVector2D& MoveInput);

	UPROPERTY(EditDefaultsOnly, Category = "Slide") float BoostSpeed     = 1600.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide") float LateralControl = 16000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide") float MinSpeed       = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float HeightOffset    = -40.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float HeightInterp    = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float MaxRollDegrees  = 3.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float RollInterpSpeed = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") int32 CameraPriority  = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Slide|Anim")
	TObjectPtr<UAnimMontage> SlideMontage;

	UPROPERTY()
	TScriptInterface<ICameraEffects> CameraEffects;

	FVector   SlideDirection = FVector::ZeroVector;
	FVector2D LastMoveInput  = FVector2D::ZeroVector;

	int32 HeightHandle = 0;
	int32 RollHandle   = 0;
};