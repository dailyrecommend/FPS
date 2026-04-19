#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "CameraManagerComponent.h"
#include "PlayerCharacterInputConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

APlayerCharacter::APlayerCharacter()
{
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(GetMesh(), NAME_None);
    Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
    Camera->bUsePawnControlRotation = true;

    CameraManager = CreateDefaultSubobject<UCameraManagerComponent>(TEXT("CameraManager"));

    bUseControllerRotationYaw   = true;
    bUseControllerRotationPitch = false;

    GetCharacterMovement()->MaxWalkSpeed  = 800.f;
    GetCharacterMovement()->JumpZVelocity = 800.f;

    GetCharacterMovement()->GroundFriction = 8.f;

    
    GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;

    GetCharacterMovement()->AirControl = 0.8f;              
    GetCharacterMovement()->AirControlBoostMultiplier = 0.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 0.f;
    GetCharacterMovement()->FallingLateralFriction = 0.3f;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    CameraManager->Initialize(Camera);
    RegisterInputMappingContext();
}

void APlayerCharacter::RegisterInputMappingContext()
{
    if (!InputConfig) return;

    const APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC) return;

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!Subsystem) return;

    if (InputConfig->IMC_KeyboardMouse)
        Subsystem->AddMappingContext(InputConfig->IMC_KeyboardMouse, 0);
    
    if (InputConfig->IMC_Gamepad)
        Subsystem->AddMappingContext(InputConfig->IMC_Gamepad, 1);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    BindInputActions(PlayerInputComponent);
}

void APlayerCharacter::BindInputActions(UInputComponent* PlayerInputComponent)
{
    if (!InputConfig) return;

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC) return;

    EIC->BindAction(InputConfig->IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Move);
    EIC->BindAction(InputConfig->IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Look);
    EIC->BindAction(InputConfig->IA_Jump, ETriggerEvent::Started,   this, &APlayerCharacter::Input_JumpStarted);
    EIC->BindAction(InputConfig->IA_Jump, ETriggerEvent::Completed,  this, &APlayerCharacter::Input_JumpCompleted);
}

void APlayerCharacter::Input_Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    if (Controller && !Axis.IsZero())
    {
        const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axis.Y);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axis.X);
    }
}

void APlayerCharacter::Input_Look(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput(Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void APlayerCharacter::Input_JumpStarted()
{
    Jump();
}

void APlayerCharacter::Input_JumpCompleted()
{
    StopJumping();
}