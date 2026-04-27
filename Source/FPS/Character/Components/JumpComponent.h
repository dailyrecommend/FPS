#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JumpComponent.generated.h"

class APlayerCharacter;
class UWallJumpComponent;
class UGlissandoComponent;

/**
 * Handles all jump-related logic:
 * - Standard jump
 * - Coyote time (grace jump after walking off a ledge)
 * - Jump buffering (queued jump while airborne, fires on landing)
 * - Wall jump delegation
 * - Glissando jump delegation
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UJumpComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UJumpComponent();

	void Initialize(APlayerCharacter* InOwner, UWallJumpComponent* InWallJump, UGlissandoComponent* InGlissando);

	/** Called on jump input press. Resolves the correct jump type. */
	void TryJump();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool CanCoyoteJump() const;
	void TickCoyoteTime(float DeltaTime);
	void TickJumpBuffer(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, Category = "Jump|CoyoteTime")
	float CoyoteTimeDuration = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Jump|Buffer")
	float JumpBufferDuration = 0.15f;

	UPROPERTY() APlayerCharacter*    OwnerCharacter = nullptr;
	UPROPERTY() UWallJumpComponent*  WallJump       = nullptr;
	UPROPERTY() UGlissandoComponent* Glissando      = nullptr;

	float CoyoteTimeCounter = 0.f;
	float JumpBufferCounter = 0.f;
	bool  bWasGrounded      = false;
	bool  bCoyoteJumpUsed   = false;
	bool  bJumpBuffered     = false;
};