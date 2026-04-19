#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallJumpComponent.generated.h"

class APlayerCharacter;

UCLASS(ClassGroup=Custom, meta=(BlueprintSpawnableComponent))
class FPS_API UWallJumpComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWallJumpComponent();

	void Initialize(APlayerCharacter* InOwner);
	bool TryWallJump();

	UFUNCTION(BlueprintPure) bool IsOnWall()          const { return bIsOnWall; }
	UFUNCTION(BlueprintPure) int32 GetWallJumpCount() const { return WallJumpCount; }
	void ResetWallJumps();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool DetectWall(FHitResult& OutHit) const;
	void ApplyWallGravity(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, Category="WallJump")
	float WallDetectionRange    = 60.f;
	UPROPERTY(EditDefaultsOnly, Category="WallJump")
	float WallJumpZVelocity     = 700.f;
	UPROPERTY(EditDefaultsOnly, Category="WallJump")
	float WallJumpLateralForce  = 600.f;
	UPROPERTY(EditDefaultsOnly, Category="WallJump")
	float WallGravityScale      = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category="WallJump")
	int32 MaxWallJumps          = 3;

	UPROPERTY() APlayerCharacter* OwnerCharacter = nullptr;

	bool    bIsOnWall      = false;
	int32   WallJumpCount  = 0;
	FVector WallNormal     = FVector::ZeroVector;
};