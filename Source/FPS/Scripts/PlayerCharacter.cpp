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
    Camera->SetRelativeLocation(FVector(0.f, 0.f, DefaultCameraHeight));
    Camera->bUsePawnControlRotation = true;

    CameraManager = CreateDefaultSubobject<UCameraManagerComponent>(TEXT("CameraManager"));

    bUseControllerRotationYaw   = true;
    bUseControllerRotationPitch = false;

    GetCharacterMovement()->MaxWalkSpeed               = DefaultMaxWalkSpeed;
    GetCharacterMovement()->JumpZVelocity              = DefaultJumpZVelocity;
    GetCharacterMovement()->GroundFriction             = DefaultGroundFriction;
    GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
    GetCharacterMovement()->AirControl                 = DefaultAirControl;
    GetCharacterMovement()->AirControlBoostMultiplier  = 0.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 0.f;
    GetCharacterMovement()->FallingLateralFriction     = DefaultFallingLateralFriction;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    CameraManager->Initialize(Camera);
    RegisterInputMappingContext();
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickGlissando(DeltaTime);
    TickDash(DeltaTime);
    CurrentMoveInput = FVector2D::ZeroVector;
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

    //= Movement & Look
    EIC->BindAction(InputConfig->IA_Move,  ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Move);
    EIC->BindAction(InputConfig->IA_Look,  ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Look);
    EIC->BindAction(InputConfig->IA_Jump,  ETriggerEvent::Started,   this, &APlayerCharacter::Input_JumpStarted);
    EIC->BindAction(InputConfig->IA_Jump,  ETriggerEvent::Completed, this, &APlayerCharacter::Input_JumpCompleted);

    //= Glissando
    EIC->BindAction(InputConfig->IA_Slide, ETriggerEvent::Started,   this, &APlayerCharacter::Input_SlideStarted);
    EIC->BindAction(InputConfig->IA_Slide, ETriggerEvent::Completed, this, &APlayerCharacter::Input_SlideCompleted);

    //= Dash
    EIC->BindAction(InputConfig->IA_Dash, ETriggerEvent::Started, this, &APlayerCharacter::Input_DashStarted);
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    DashChargeTimer = 0.f;
}

void APlayerCharacter::Input_Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    CurrentMoveInput = Axis;

    if (bIsGlissando) return;

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
    if (bIsGlissando)
    {
        bIsGlissando = false;
        Jump();
        Camera->SetRelativeLocation(FVector(0.f, 0.f, DefaultCameraHeight));
        CurrentCameraRoll = 0.f;
        APlayerController* PC = Cast<APlayerController>(Controller);
        if (PC)
        {
            FRotator ControlRot = PC->GetControlRotation();
            ControlRot.Roll = 0.f;
            PC->SetControlRotation(ControlRot);
        }
        GetCharacterMovement()->GroundFriction             = DefaultGroundFriction;
        GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
    }
    else
    {
        Jump();
    }
}

void APlayerCharacter::Input_JumpCompleted()
{
    StopJumping();
}

void APlayerCharacter::Input_SlideStarted()
{
    bWantsGlissando = true;
    if (CanGlissando()) StartGlissando();
}

void APlayerCharacter::Input_SlideCompleted()
{
    bWantsGlissando = false;
    EndGlissando();
}

bool APlayerCharacter::CanGlissando() const
{
    return GetCharacterMovement()->IsMovingOnGround()
        && GetVelocity().Size2D() > GlissandoMinSpeed
        && !bIsGlissando;
}

void APlayerCharacter::StartGlissando()
{
    bIsGlissando = true;

    GlissandoDirection = GetActorForwardVector();
    GlissandoDirection.Z = 0.f;
    GlissandoDirection.Normalize();

    GetCharacterMovement()->Velocity               = GlissandoDirection * GlissandoBoostSpeed;
    GetCharacterMovement()->GroundFriction         = 0.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 0.f;
    Camera->SetRelativeLocation(FVector(0.f, 0.f, DefaultCameraHeight + GlissandoCameraHeight));
}

void APlayerCharacter::EndGlissando()
{
    if (!bIsGlissando) return;
    bIsGlissando = false;

    GetCharacterMovement()->GroundFriction             = DefaultGroundFriction;
    GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
    Camera->SetRelativeLocation(FVector(0.f, 0.f, DefaultCameraHeight));
    CurrentCameraRoll = 0.f;

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (PC)
    {
        FRotator ControlRot = PC->GetControlRotation();
        ControlRot.Roll = 0.f;
        PC->SetControlRotation(ControlRot);
    }
}

void APlayerCharacter::TickGlissando(float DeltaTime)
{
    if (!bIsGlissando) return;

    float CurrentSpeed = GetVelocity().Size2D();

    if (CurrentSpeed < GlissandoMinSpeed)
    {
        EndGlissando();
        return;
    }

    FVector GlissandoRight = FVector::CrossProduct(FVector::UpVector, GlissandoDirection);
    GlissandoRight.Normalize();

    FVector LateralInput   = GlissandoRight * CurrentMoveInput.X * GlissandoLateralControl * DeltaTime;
    FVector TargetVelocity = GlissandoDirection * GlissandoBoostSpeed + LateralInput;
    TargetVelocity.Z       = GetCharacterMovement()->Velocity.Z;
    GetCharacterMovement()->Velocity = TargetVelocity;

    float TargetRoll  = CurrentMoveInput.X * GlissandoCameraRoll;
    CurrentCameraRoll = FMath::FInterpTo(CurrentCameraRoll, TargetRoll, DeltaTime, GlissandoCameraRollInterpSpeed);

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (PC)
    {
        FRotator ControlRot = PC->GetControlRotation();
        ControlRot.Roll     = CurrentCameraRoll;
        PC->SetControlRotation(ControlRot);
    }
}

//= ====================Dash
void APlayerCharacter::Input_DashStarted()
{
    if (DashCharges <= 0 || bIsDashing) return;
    PerformDash();
}

void APlayerCharacter::PerformDash()
{
    bIsDashing  = true;
    DashElapsed = 0.f;
    DashCharges--;
    
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (PC)
    {
        FRotator CamRot = PC->GetControlRotation();
        DashDirection   = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X);
        DashDirection.Normalize();
    }

    if (bIsGlissando) EndGlissando();

    GetCharacterMovement()->Velocity = DashDirection * DashSpeed;
    
    bDashChargeDelay = true;
    DashDelayTimer   = 0.f;
}


void APlayerCharacter::TickDash(float DeltaTime)
{
    if (bIsDashing)
    {
        DashElapsed += DeltaTime;
        GetCharacterMovement()->Velocity = DashDirection * DashSpeed;

        if (DashElapsed >= DashDuration)
        {
            bIsDashing = false;

            if (GetCharacterMovement()->IsMovingOnGround())
            {
                GetCharacterMovement()->Velocity = FVector::ZeroVector;
            }
            else
            {
                GetCharacterMovement()->Velocity = DashDirection * DefaultMaxWalkSpeed;
            }
        }
    }

    if (bDashChargeDelay)
    {
        DashDelayTimer += DeltaTime;
        if (DashDelayTimer >= DashChargeDelay)
        {
            bDashChargeDelay = false;
            DashChargeTimer  = 0.f;
        }
        return;
    }

    if (DashCharges < MaxDashCharges && GetCharacterMovement()->IsMovingOnGround())
    {
        DashChargeTimer += DeltaTime;
        if (DashChargeTimer >= DashChargeInterval)
        {
            DashChargeTimer = 0.f;
            AddDashCharge();
        }
    }
}

void APlayerCharacter::AddDashCharge()
{
    DashCharges = FMath::Min(DashCharges + 1, MaxDashCharges);
}

void APlayerCharacter::AddDashChargeImmediate()
{
    AddDashCharge();

    bDashChargeDelay = false;
    DashChargeTimer  = 0.f;
}