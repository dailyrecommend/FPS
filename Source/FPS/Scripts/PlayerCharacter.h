#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UCameraManagerComponent;
class UPlayerCharacterInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class FPS_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UPlayerCharacterInputConfig* InputConfig;

private:
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_JumpStarted();
	void Input_JumpCompleted();

	void BindInputActions(UInputComponent* PlayerInputComponent);
	void RegisterInputMappingContext();

	UPROPERTY(VisibleAnywhere, Category="Camera")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category="Camera")
	UCameraManagerComponent* CameraManager;
};