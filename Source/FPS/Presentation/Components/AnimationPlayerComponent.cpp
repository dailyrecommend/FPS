#include "Presentation/Components/AnimationPlayerComponent.h"
#include "Presentation/AnimInstance/FPSAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"

void UAnimationPlayerComponent::InjectMesh(USkeletalMeshComponent* InMesh)
{
    Mesh = InMesh;
}

UFPSAnimInstance* UAnimationPlayerComponent::GetFPSAnimInstance() const
{
    USkeletalMeshComponent* M = Mesh.Get();
    if (!M) return nullptr;
    return Cast<UFPSAnimInstance>(M->GetAnimInstance());
}

void UAnimationPlayerComponent::PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
    if (!Montage) return;
    UFPSAnimInstance* Anim = GetFPSAnimInstance();
    if (!Anim) return;
    Anim->Montage_Play(Montage, PlayRate <= 0.f ? 1.f : PlayRate);
}

void UAnimationPlayerComponent::PlayMontageSection_Implementation(UAnimMontage* Montage, FName SectionName, float PlayRate)
{
    if (!Montage) return;
    UFPSAnimInstance* Anim = GetFPSAnimInstance();
    if (!Anim) return;

    Anim->Montage_Play(Montage, PlayRate <= 0.f ? 1.f : PlayRate);
    Anim->Montage_JumpToSection(SectionName, Montage);
}

void UAnimationPlayerComponent::StopMontage_Implementation(UAnimMontage* Montage, float BlendOutTime)
{
    if (!Montage) return;
    UFPSAnimInstance* Anim = GetFPSAnimInstance();
    if (!Anim) return;
    Anim->Montage_Stop(FMath::Max(BlendOutTime, 0.f), Montage);
}

bool UAnimationPlayerComponent::IsMontagePlaying_Implementation(UAnimMontage* Montage) const
{
    if (!Montage) return false;
    UFPSAnimInstance* Anim = GetFPSAnimInstance();
    if (!Anim) return false;
    return Anim->Montage_IsPlaying(Montage);
}

void UAnimationPlayerComponent::SetLocomotionState(float Speed, bool bInAir, int32 WeaponType)
{
    UFPSAnimInstance* Anim = GetFPSAnimInstance();
    if (!Anim) return;
    Anim->SetLocomotionState(Speed, bInAir, (float)WeaponType);
}

void UAnimationPlayerComponent::SetWeaponType(int32 WeaponType)
{
    CurrentWeaponType = WeaponType;

    UFPSAnimInstance* Anim = GetFPSAnimInstance();
    if (!Anim) return;
    Anim->WeaponType = (float)WeaponType;
}