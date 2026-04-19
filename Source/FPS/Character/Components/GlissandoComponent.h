#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GlissandoComponent.generated.h"

class UCameraComponent;
class APlayerCharacter;

UCLASS(ClassGroup=Custom, meta=(BlueprintSpawnableComponent))
class FPS_API UGlissandoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGlissandoComponent();

	void Initialize(APlayerCharacter* InOwner, UCameraComponent* InCamera);

	void StartGlissando();
	void EndGlissando();
	bool CanGlissando() const;
	void OnMoveInput(FVector2D Input);

	UFUNCTION(BlueprintPure) bool IsGlissando() const { return bIsGlissando; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	void TickGlissando(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, Category="Glissando")
	float GlissandoBoostSpeed           = 1600.f;
	UPROPERTY(EditDefaultsOnly, Category="Glissando")
	float GlissandoLateralControl       = 16000.f;
	UPROPERTY(EditDefaultsOnly, Category="Glissando")
	float GlissandoCameraHeight         = -40.f;
	UPROPERTY(EditDefaultsOnly, Category="Glissando")
	float GlissandoCameraRoll           = 3.f;
	UPROPERTY(EditDefaultsOnly, Category="Glissando")
	float GlissandoMinSpeed             = 100.f;
	UPROPERTY(EditDefaultsOnly, Category="Glissando")
	float GlissandoCameraRollInterpSpeed = 8.f;

	UPROPERTY() APlayerCharacter* OwnerCharacter  = nullptr;
	UPROPERTY() UCameraComponent* Camera          = nullptr;

	bool      bIsGlissando       = false;
	float     CurrentCameraRoll  = 0.f;
	FVector   GlissandoDirection = FVector::ZeroVector;
	FVector2D CurrentMoveInput   = FVector2D::ZeroVector;
};