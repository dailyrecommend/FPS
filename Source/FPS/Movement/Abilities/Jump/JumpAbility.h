#pragma once
#include "CoreMinimal.h"
#include "Movement/Base/AbilityBase.h"
#include "JumpAbility.generated.h"

class UAbilityRegistry;

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UJumpAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	UJumpAbility();

	UPROPERTY(EditDefaultsOnly, Category = "Jump|Interactions")
	FName SlideAbilityId = TEXT("Slide");

	UPROPERTY(EditDefaultsOnly, Category = "Jump|Interactions")
	FName WallJumpAbilityId = TEXT("WallJump");

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	virtual EActivationResult OnTryActivate(const FAbilityContext& Context) override;

private:
	UAbilityRegistry* GetRegistry() const;

	bool TrySlideExitJump();
	bool TryGroundJump();
	bool TryDelegateWallJump(const FAbilityContext& Context);
	bool TryCoyoteJump();
	void BufferJump();

	void TickCoyoteTime(float DeltaTime);
	void TickJumpBuffer(float DeltaTime);

	bool CanCoyoteJump() const;

	UPROPERTY(EditDefaultsOnly, Category = "Jump|CoyoteTime") float CoyoteTimeDuration = 0.15f;
	UPROPERTY(EditDefaultsOnly, Category = "Jump|Buffer")     float JumpBufferDuration = 0.15f;

	float CoyoteTimeCounter = 0.f;
	float JumpBufferCounter = 0.f;
	bool  bWasGrounded      = false;
	bool  bCoyoteJumpUsed   = false;
	bool  bJumpBuffered     = false;
};