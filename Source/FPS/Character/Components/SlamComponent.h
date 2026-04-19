#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SlamComponent.generated.h"

class APlayerCharacter;

UCLASS(ClassGroup=Custom, meta=(BlueprintSpawnableComponent))
class FPS_API USlamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USlamComponent();

	void Initialize(APlayerCharacter* InOwner);
	bool TrySlam();
	void CancelSlam();

	UFUNCTION(BlueprintPure) bool IsSlamming() const { return bIsSlamming; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	void OnSlamLanded();

	UPROPERTY(EditDefaultsOnly, Category="Slam")
	float SlamDownForce     = 4000.f;
	UPROPERTY(EditDefaultsOnly, Category="Slam")
	float SlamLandingImpact = 1000.f;  // 나중에 무기별 효과 연결용
	UPROPERTY(EditDefaultsOnly, Category="Slam")
	float SlamLandingStunDuration = 0.1f;
	
	UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;

	bool bIsSlamming = false;
	bool  bIsStunned    = false;
	float StunTimer     = 0.f;
};