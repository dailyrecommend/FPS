#pragma once
#include "CoreMinimal.h"
#include "Movement/Base/AbilityBase.h"
#include "WallJumpAbility.generated.h"

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UWallJumpAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	UWallJumpAbility();

	UFUNCTION(BlueprintCallable) void  ResetWallJumps();
	UFUNCTION(BlueprintPure)     bool  IsOnWall()         const { return bIsOnWall; }
	UFUNCTION(BlueprintPure)     int32 GetWallJumpCount() const { return WallJumpCount; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	virtual EActivationResult OnTryActivate(const FAbilityContext& Context) override;
	virtual bool              CheckPreconditions(const FAbilityContext& Context) const override;

private:
	bool DetectWall(FHitResult& OutHit) const;
	void TickWallState();

	UPROPERTY(EditDefaultsOnly, Category = "WallJump") float WallDetectionRange   = 60.f;
	UPROPERTY(EditDefaultsOnly, Category = "WallJump") float WallJumpZVelocity    = 700.f;
	UPROPERTY(EditDefaultsOnly, Category = "WallJump") float WallJumpLateralForce = 600.f;
	UPROPERTY(EditDefaultsOnly, Category = "WallJump") float WallGravityScale     = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category = "WallJump") int32 MaxWallJumps         = 3;

	bool    bIsOnWall     = false;
	int32   WallJumpCount = 0;
	FVector WallNormal    = FVector::ZeroVector;
};