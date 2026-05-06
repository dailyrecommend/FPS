#pragma once
#include "CoreMinimal.h"
#include "Movement/Base/AbilityBase.h"
#include "Presentation/Interfaces/CameraEffects.h"
#include "GlissandoAbility.generated.h"

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UGlissandoAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	UGlissandoAbility();

	void AttachCameraEffects(TScriptInterface<ICameraEffects> InEffects) { CameraEffects = InEffects; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	virtual EActivationResult OnTryActivate(const FAbilityContext& Context) override;
	virtual void              OnDeactivate() override;
	virtual bool              CheckPreconditions(const FAbilityContext& Context) const override;

private:
	void TickGlissando(float DeltaTime, const FVector2D& MoveInput);

	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float BoostSpeed     = 1600.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float LateralControl = 16000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float MinSpeed       = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Glissando|Camera") float HeightOffset    = -40.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando|Camera") float HeightInterp    = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando|Camera") float MaxRollDegrees  = 3.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando|Camera") float RollInterpSpeed = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando|Camera") int32 CameraPriority  = 1;

	UPROPERTY()
	TScriptInterface<ICameraEffects> CameraEffects;

	FVector   GlissandoDirection = FVector::ZeroVector;
	FVector2D LastMoveInput      = FVector2D::ZeroVector;

	int32 HeightHandle = 0;
	int32 RollHandle   = 0;
};