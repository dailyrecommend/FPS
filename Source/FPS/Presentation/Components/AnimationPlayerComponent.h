#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Presentation/Interfaces/AnimationPlayer.h"
#include "AnimationPlayerComponent.generated.h"

class USkeletalMeshComponent;

/**
 * Default IAnimationPlayer implementation that plays montages on a target mesh
 * (typically the FP arms mesh).
 *
 * The mesh is injected — the component never reaches into the owner to find it,
 * so the same component can be used with any mesh layout.
 */
UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UAnimationPlayerComponent
	: public UActorComponent
	, public IAnimationPlayer
{
	GENERATED_BODY()

public:
	void InjectMesh(USkeletalMeshComponent* InMesh);

	virtual void PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate) override;
	virtual void StopMontage_Implementation(UAnimMontage* Montage, float BlendOutTime) override;
	virtual bool IsMontagePlaying_Implementation(UAnimMontage* Montage) const override;

private:
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;
};