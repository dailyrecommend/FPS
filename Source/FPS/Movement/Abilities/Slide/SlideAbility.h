#pragma once
#include "CoreMinimal.h"
#include "Movement/Base/AbilityBase.h"
#include "Presentation/Interfaces/CameraEffects.h"
#include "SlideAbility.generated.h"

class UWeaponRegistry;
class UPlayerInputRouter;

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USlideAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	USlideAbility();

	void AttachCameraEffects(TScriptInterface<ICameraEffects> InEffects) { CameraEffects = InEffects; }
	void AttachWeaponRegistry(UWeaponRegistry* InRegistry)              { WeaponRegistry = InRegistry; }
	void AttachInputRouter(UPlayerInputRouter* InRouter)                { InputRouter = InRouter; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	virtual EActivationResult OnTryActivate(const FAbilityContext& Context) override;
	virtual void              OnDeactivate() override;
	virtual bool              CheckPreconditions(const FAbilityContext& Context) const override;

private:
	void TickSlide(float DeltaTime, const FVector2D& MoveInput);

	UPROPERTY(EditDefaultsOnly, Category = "Slide") float BoostSpeed          = 1600.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide") float LateralControl      = 16000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide") float MinSpeed            = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide") float CapsuleHeightOffset = -40.f;

	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float HeightOffset    = -40.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float HeightInterp    = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float MaxRollDegrees  = 3.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float RollInterpSpeed = 8.f;

	/** A/D 입력을 한 번 더 보간해 시작도 완만하게 만든다(ease-in). 낮을수록 천천히 붙는다. */
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") float RollInputSmoothSpeed = 6.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide|Camera") int32 CameraPriority  = 1;

	UPROPERTY()
	TScriptInterface<ICameraEffects> CameraEffects;

	UPROPERTY()
	TObjectPtr<UWeaponRegistry> WeaponRegistry;

	UPROPERTY()
	TObjectPtr<UPlayerInputRouter> InputRouter;

	FVector SlideDirection           = FVector::ZeroVector;
	float   DefaultCapsuleHalfHeight = 88.f;
	float   SmoothedRollInput        = 0.f;

	int32 HeightHandle = 0;
	int32 RollHandle   = 0;
};