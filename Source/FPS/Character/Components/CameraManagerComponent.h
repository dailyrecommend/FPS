#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraManagerComponent.generated.h"

class UCameraComponent;

UCLASS(ClassGroup=Custom, meta=(BlueprintSpawnableComponent))
class FPS_API UCameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraManagerComponent();

	void Initialize(UCameraComponent* InCamera);

	UFUNCTION(BlueprintPure)
	UCameraComponent* GetCamera() const { return Camera; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UCameraComponent* Camera;
};