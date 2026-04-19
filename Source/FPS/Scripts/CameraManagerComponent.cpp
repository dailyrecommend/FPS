#include "CameraManagerComponent.h"
#include "Camera/CameraComponent.h"

UCameraManagerComponent::UCameraManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCameraManagerComponent::Initialize(UCameraComponent* InCamera)
{
	Camera = InCamera;
}

void UCameraManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}