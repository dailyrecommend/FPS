#include "SlamComponent.h"
#include "../PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

USlamComponent::USlamComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USlamComponent::Initialize(APlayerCharacter* InOwner)
{
	OwnerCharacter = InOwner;
}

bool USlamComponent::TrySlam()
{
	if (!OwnerCharacter) return false;
	if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround()) return false;
	if (bIsSlamming) return false;

	bIsSlamming = true;

	// 수평 속도 제거 후 아래로 강한 힘
	FVector SlamVelocity = FVector(0.f, 0.f, -SlamDownForce);
	OwnerCharacter->GetCharacterMovement()->Velocity = SlamVelocity;

	return true;
}

void USlamComponent::CancelSlam()
{
	if (!bIsSlamming) return;
	bIsSlamming = false;
	bIsStunned  = false;
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void USlamComponent::OnSlamLanded()
{
	bIsSlamming = false;
	bIsStunned  = true;
	StunTimer   = SlamLandingStunDuration;

	OwnerCharacter->GetCharacterMovement()->DisableMovement();
}

void USlamComponent::TickComponent(float DeltaTime, ELevelTick TickType,
									FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsSlamming && OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		OnSlamLanded();
		return;
	}

	if (bIsStunned)
	{
		StunTimer -= DeltaTime;
		if (StunTimer <= 0.f)
		{
			bIsStunned = false;
			OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
}