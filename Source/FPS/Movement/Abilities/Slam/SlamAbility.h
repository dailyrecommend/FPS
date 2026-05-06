#pragma once
#include "CoreMinimal.h"
#include "Movement/Base/AbilityBase.h"
#include "SlamAbility.generated.h"

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API USlamAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	USlamAbility();

	UFUNCTION(BlueprintPure) bool IsStunned() const { return bIsStunned; }

	UPROPERTY(EditDefaultsOnly, Category = "Slam|Interactions")
	FName DashAbilityIdAsInterruptor = TEXT("Dash");

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	virtual EActivationResult OnTryActivate(const FAbilityContext& Context) override;
	virtual void              OnDeactivate() override;
	virtual bool              CheckPreconditions(const FAbilityContext& Context) const override;
	virtual bool              CanBeInterruptedBy_Implementation(FName OtherAbilityId) const override;

private:
	void OnSlamLanded();
	void TickStun(float DeltaTime);
	void RestoreMovementMode();

	UPROPERTY(EditDefaultsOnly, Category = "Slam") float SlamDownForce           = 4000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slam") float SlamLandingImpact       = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slam") float SlamLandingStunDuration = 0.1f;

	bool  bIsStunned = false;
	float StunTimer  = 0.f;
};