#pragma once
#include "CoreMinimal.h"
#include "Movement/Base/AbilityBase.h"
#include "GlissandoAbility.generated.h"

class UCameraComponent;

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UGlissandoAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	UGlissandoAbility();

	/** Camera dependency injected separately because cameras are not part of ACharacter. */
	void InjectCamera(UCameraComponent* InCamera);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	virtual EActivationResult OnTryActivate(const FAbilityContext& Context) override;
	virtual void              OnDeactivate() override;
	virtual bool              CheckPreconditions(const FAbilityContext& Context) const override;

private:
	void TickGlissando(float DeltaTime, const FVector2D& MoveInput);
	void ApplyCameraRoll(float Roll);
	void ResetCameraRoll();

	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float BoostSpeed            = 1600.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float LateralControl        = 16000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float CameraHeight          = -40.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float CameraRoll            = 3.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float MinSpeed              = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Glissando") float CameraRollInterpSpeed = 8.f;

	UPROPERTY()
	TWeakObjectPtr<UCameraComponent> Camera;

	FVector   GlissandoDirection = FVector::ZeroVector;
	float     CurrentCameraRoll  = 0.f;
	FVector2D LastMoveInput      = FVector2D::ZeroVector;
};