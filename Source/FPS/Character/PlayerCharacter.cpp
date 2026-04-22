#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CameraManagerComponent.h"
#include "Components/GlissandoComponent.h"
#include "Components/DashComponent.h"
#include "Components/WallJumpComponent.h"
#include "Components/SlamComponent.h"
#include "Components/GunComponent.h"
#include "Components/TimeScaleComponent.h"
#include "../Input/PlayerCharacterInputConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(GetMesh(), NAME_None);
    Camera->SetFieldOfView(DefaultFOV);
    Camera->SetRelativeLocation(FVector(0.f, 0.f, DefaultCameraHeight));
    Camera->bUsePawnControlRotation = true;

    CameraManager   = CreateDefaultSubobject<UCameraManagerComponent>(TEXT("CameraManager"));
    Glissando       = CreateDefaultSubobject<UGlissandoComponent>(TEXT("Glissando"));
    Dash            = CreateDefaultSubobject<UDashComponent>(TEXT("Dash"));
    WallJump        = CreateDefaultSubobject<UWallJumpComponent>(TEXT("WallJump"));
    Slam            = CreateDefaultSubobject<USlamComponent>(TEXT("Slam"));
    Gun             = CreateDefaultSubobject<UGunComponent>(TEXT("Gun"));
    ArmsMesh        = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
    TimeScale       = CreateDefaultSubobject<UTimeScaleComponent>(TEXT("TimeScale"));
    
    bUseControllerRotationYaw   = true;
    bUseControllerRotationPitch = false;

    GetCharacterMovement()->MaxWalkSpeed                = DefaultMaxWalkSpeed;
    GetCharacterMovement()->JumpZVelocity               = DefaultJumpZVelocity;
    GetCharacterMovement()->GroundFriction              = DefaultGroundFriction;
    GetCharacterMovement()->BrakingDecelerationWalking  = DefaultBrakingDeceleration;
    GetCharacterMovement()->AirControl                  = DefaultAirControl;
    GetCharacterMovement()->AirControlBoostMultiplier   = 0.f;
    GetCharacterMovement()->BrakingDecelerationFalling  = 0.f;
    GetCharacterMovement()->FallingLateralFriction      = DefaultFallingLateralFriction;
    
    ArmsMesh->SetupAttachment(Camera);
    ArmsMesh->SetOnlyOwnerSee(true);
    ArmsMesh->bCastDynamicShadow = false;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    CameraManager   ->Initialize(Camera);
    Glissando       ->Initialize(this, Camera);
    Dash            ->Initialize(this);
    WallJump        ->Initialize(this);
    Slam            ->Initialize(this);
    Gun             ->Initialize(this, Camera);
    ArmsMesh        ->SetupAttachment(Camera);
    TimeScale       ->Initialize(this);
    
    RegisterInputMappingContext();
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickCoyoteTime(DeltaTime);
    TickJumpBuffer(DeltaTime);
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

    EIC->BindAction(InputConfig->IA_Move,  ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Move);
    EIC->BindAction(InputConfig->IA_Look,  ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Look);
    EIC->BindAction(InputConfig->IA_Jump,  ETriggerEvent::Started,   this, &APlayerCharacter::Input_JumpStarted);
    EIC->BindAction(InputConfig->IA_Jump,  ETriggerEvent::Completed, this, &APlayerCharacter::Input_JumpCompleted);
    EIC->BindAction(InputConfig->IA_Slide, ETriggerEvent::Started,   this, &APlayerCharacter::Input_SlideStarted);
    EIC->BindAction(InputConfig->IA_Slide, ETriggerEvent::Completed, this, &APlayerCharacter::Input_SlideCompleted);
    EIC->BindAction(InputConfig->IA_Dash,  ETriggerEvent::Started,   this, &APlayerCharacter::Input_DashStarted);
    EIC->BindAction(InputConfig->IA_Slam, ETriggerEvent::Started, this, &APlayerCharacter::Input_SlamStarted);
    EIC->BindAction(InputConfig->IA_Attack, ETriggerEvent::Triggered, this, &APlayerCharacter::Input_AttackStarted);
    EIC->BindAction(InputConfig->IA_WeaponSkill, ETriggerEvent::Started,   this, &APlayerCharacter::Input_WeaponSkillStarted);
    EIC->BindAction(InputConfig->IA_WeaponSkill, ETriggerEvent::Completed, this, &APlayerCharacter::Input_WeaponSkillCompleted);
    
}

void APlayerCharacter::Input_Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    Glissando->OnMoveInput(Axis);

    if (Glissando->IsGlissando()) return;

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
    float Sensitivity    = GetLookSensitivityMultiplier();
    AddControllerYawInput(Axis.X * Sensitivity);
    AddControllerPitchInput(Axis.Y * Sensitivity);
}
void APlayerCharacter::Input_JumpStarted()
{
    if (Glissando->IsGlissando())
    {
        Glissando->EndGlissando();
        Jump();
        GetCharacterMovement()->GroundFriction             = DefaultGroundFriction;
        GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
    }
    else if (GetCharacterMovement()->IsMovingOnGround())
    {
        Jump();
    }
    else if (WallJump->IsOnWall() && WallJump->TryWallJump())
    {
        // TryWallJump 안에서 처리
    }
    else if (CanCoyoteJump())
    {
        bCoyoteJumpUsed   = true;
        CoyoteTimeCounter = 0.f;
        Jump();
    }
    else
    {
        bJumpBuffered     = true;
        JumpBufferCounter = JumpBufferDuration;
    }
}

void APlayerCharacter::Input_JumpCompleted()
{
    StopJumping();
}

void APlayerCharacter::Input_SlideStarted()
{
    if (Glissando->CanGlissando()) Glissando->StartGlissando();
}

void APlayerCharacter::Input_SlideCompleted()
{
    Glissando->EndGlissando();
}

void APlayerCharacter::Input_DashStarted()
{
    if (Slam->IsSlamming()) Slam->CancelSlam();
    Dash->TryDash();
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    WallJump->ResetWallJumps();
}

void APlayerCharacter::TickCoyoteTime(float DeltaTime)
{
    bool bIsGrounded = GetCharacterMovement()->IsMovingOnGround();
    
    if (bWasGrounded && !bIsGrounded)
    {
        CoyoteTimeCounter = CoyoteTimeThreshold;
        bCoyoteJumpUsed   = false;
    }

    if (!bIsGrounded && CoyoteTimeCounter > 0.f)
        CoyoteTimeCounter -= DeltaTime;
    
    if (bIsGrounded)
    {
        CoyoteTimeCounter = 0.f;
        bCoyoteJumpUsed   = false;
    }

    bWasGrounded = bIsGrounded;
}

void APlayerCharacter::TickJumpBuffer(float DeltaTime)
{
    if (bJumpBuffered)
    {
        JumpBufferCounter -= DeltaTime;
        
        if (GetCharacterMovement()->IsMovingOnGround())
        {
            Jump();
            bJumpBuffered     = false;
            JumpBufferCounter = 0.f;
        }
        
        if (JumpBufferCounter <= 0.f)
        {
            bJumpBuffered     = false;
            JumpBufferCounter = 0.f;
        }
    }
}

bool APlayerCharacter::CanCoyoteJump() const
{
    return !GetCharacterMovement()->IsMovingOnGround()
        && CoyoteTimeCounter > 0.f
        && !bCoyoteJumpUsed;
}

void APlayerCharacter::Input_SlamStarted()
{
    Slam->TrySlam();
}

void APlayerCharacter::Input_AttackStarted()
{
    Gun->TryFire();
}

void APlayerCharacter::Input_WeaponSkillStarted()
{
    Gun->StartFocus();
}

void APlayerCharacter::Input_WeaponSkillCompleted()
{
    Gun->EndFocus();
}

float APlayerCharacter::GetLookSensitivityMultiplier() const
{
    if (Gun && Gun->IsFocusing())
        return Gun->FocusSensitivity;
    return 1.f;
}