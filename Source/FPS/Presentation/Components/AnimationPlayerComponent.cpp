#include "Presentation/Components/AnimationPlayerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

void UAnimationPlayerComponent::InjectMesh(USkeletalMeshComponent* InMesh)
{
	Mesh = InMesh;
}

void UAnimationPlayerComponent::PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage) return;

	USkeletalMeshComponent* M = Mesh.Get();
	if (!M) return;

	UAnimInstance* Anim = M->GetAnimInstance();
	if (!Anim) return;

	Anim->Montage_Play(Montage, PlayRate <= 0.f ? 1.f : PlayRate);
}

void UAnimationPlayerComponent::StopMontage_Implementation(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage) return;

	USkeletalMeshComponent* M = Mesh.Get();
	if (!M) return;

	UAnimInstance* Anim = M->GetAnimInstance();
	if (!Anim) return;

	Anim->Montage_Stop(FMath::Max(BlendOutTime, 0.f), Montage);
}

bool UAnimationPlayerComponent::IsMontagePlaying_Implementation(UAnimMontage* Montage) const
{
	if (!Montage) return false;

	USkeletalMeshComponent* M = Mesh.Get();
	if (!M) return false;

	UAnimInstance* Anim = M->GetAnimInstance();
	if (!Anim) return false;

	return Anim->Montage_IsPlaying(Montage);
}