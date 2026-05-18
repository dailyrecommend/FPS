#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Presentation/Interfaces/AnimationPlayer.h"
#include "AnimationPlayerComponent.generated.h"

class USkeletalMeshComponent;
class UFPSAnimInstance;

UCLASS(ClassGroup = Custom, meta = (BlueprintSpawnableComponent))
class FPS_API UAnimationPlayerComponent
	: public UActorComponent
	, public IAnimationPlayer
{
	GENERATED_BODY()

public:
	void InjectMesh(USkeletalMeshComponent* InMesh);

	virtual void PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate) override;
	virtual void PlayMontageSection_Implementation(UAnimMontage* Montage, FName SectionName, float PlayRate) override;
	virtual void StopMontage_Implementation(UAnimMontage* Montage, float BlendOutTime) override;
	virtual bool IsMontagePlaying_Implementation(UAnimMontage* Montage) const override;

	void SetLocomotionState(float Speed, bool bInAir, int32 WeaponType);
	void SetWeaponType(int32 WeaponType);
	USkeletalMeshComponent* GetMesh() const { return Mesh.Get(); }
	
	int32 GetCurrentWeaponType() const { return CurrentWeaponType; }

private:
	UFPSAnimInstance* GetFPSAnimInstance() const;

	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;

	int32 CurrentWeaponType = 0;
};