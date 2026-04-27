#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CameraManagerComponent.h"
#include "Components/GlissandoComponent.h"
#include "Components/DashComponent.h"
#include "Components/WallJumpComponent.h"
#include "Components/SlamComponent.h"
#include "Components/GunComponent.h"
#include "Components/FocusComponent.h"
#include "Components/TimeScaleComponent.h"
#include "Components/WeaponSwapComponent.h"
#include "Components/JumpComponent.h"
#include "../Input/PlayerCharacterInputConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Camera — GetMesh()에 attach. Transform은 BP에서 관리
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(GetMesh());
    Camera->bUsePawnControlRotation = true;

    // ArmsMesh — Camera에 attach. Transform은 BP에서 관리
    ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
    ArmsMesh->SetupAttachment(Camera);
    ArmsMesh->SetOnlyOwnerSee(true);
    ArmsMesh->bCastDynamicShadow = false;

    // 무기 메쉬 — ArmsMesh에 attach. Transform은 BP에서 관리
    GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponGunMesh"));
    GunMesh->SetupAttachment(ArmsMesh);

    SwordMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSwordMesh"));
    SwordMesh->SetupAttachment(ArmsMesh);

    // Components
    CameraManager = CreateDefaultSubobject<UCameraManagerComponent>(TEXT("CameraManager"));
    Glissando     = CreateDefaultSubobject<UGlissandoComponent>(TEXT("Glissando"));
    Dash          = CreateDefaultSubobject<UDashComponent>(TEXT("Dash"));
    WallJump      = CreateDefaultSubobject<UWallJumpComponent>(TEXT("WallJump"));
    Slam          = CreateDefaultSubobject<USlamComponent>(TEXT("Slam"));
    JumpComp      = CreateDefaultSubobject<UJumpComponent>(TEXT("Jump"));
    Gun           = CreateDefaultSubobject<UGunComponent>(TEXT("Gun"));
    Focus         = CreateDefaultSubobject<UFocusComponent>(TEXT("Focus"));
    TimeScale     = CreateDefaultSubobject<UTimeScaleComponent>(TEXT("TimeScale"));
    WeaponSwap    = CreateDefaultSubobject<UWeaponSwapComponent>(TEXT("WeaponSwap"));

    bUseControllerRotationYaw   = true;
    bUseControllerRotationPitch = false;

    SetupMovementDefaults();
}

void APlayerCharacter::SetupMovementDefaults()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed                = DefaultMaxWalkSpeed;
    MoveComp->JumpZVelocity               = DefaultJumpZVelocity;
    MoveComp->GroundFriction              = DefaultGroundFriction;
    MoveComp->BrakingDecelerationWalking  = DefaultBrakingDeceleration;
    MoveComp->AirControl                  = DefaultAirControl;
    MoveComp->AirControlBoostMultiplier   = 0.f;
    MoveComp->BrakingDecelerationFalling  = 0.f;
    MoveComp->FallingLateralFriction      = DefaultFallingLateralFriction;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponents();
    RegisterInputMappingContext();
}

void APlayerCharacter::InitializeComponents()
{
    CameraManager->Initialize(Camera);
    Glissando->Initialize(this, Camera);
    Dash->Initialize(this);
    WallJump->Initialize(this);
    Slam->Initialize(this);
    JumpComp->Initialize(this, WallJump, Glissando);
    Gun->Initialize(this, Camera);
    Focus->Initialize(this, Camera, Gun);
    TimeScale->Initialize(this);
    WeaponSwap->Initialize(this, GunMesh, SwordMesh);
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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

    EIC->BindAction(InputConfig->IA_Move,             ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Move);
    EIC->BindAction(InputConfig->IA_Look,             ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Look);
    EIC->BindAction(InputConfig->IA_Jump,             ETriggerEvent::Started,   this, &APlayerCharacter::Input_JumpStarted);
    EIC->BindAction(InputConfig->IA_Jump,             ETriggerEvent::Completed, this, &APlayerCharacter::Input_JumpCompleted);
    EIC->BindAction(InputConfig->IA_Slide,            ETriggerEvent::Started,   this, &APlayerCharacter::Input_SlideStarted);
    EIC->BindAction(InputConfig->IA_Slide,            ETriggerEvent::Completed, this, &APlayerCharacter::Input_SlideCompleted);
    EIC->BindAction(InputConfig->IA_Dash,             ETriggerEvent::Started,   this, &APlayerCharacter::Input_DashStarted);
    EIC->BindAction(InputConfig->IA_Slam,             ETriggerEvent::Started,   this, &APlayerCharacter::Input_SlamStarted);
    EIC->BindAction(InputConfig->IA_Attack,           ETriggerEvent::Triggered, this, &APlayerCharacter::Input_AttackStarted);
    EIC->BindAction(InputConfig->IA_WeaponSkill,      ETriggerEvent::Started,   this, &APlayerCharacter::Input_WeaponSkillStarted);
    EIC->BindAction(InputConfig->IA_WeaponSkill,      ETriggerEvent::Completed, this, &APlayerCharacter::Input_WeaponSkillCompleted);
    EIC->BindAction(InputConfig->IA_WeaponSwapGun,    ETriggerEvent::Started,   this, &APlayerCharacter::Input_WeaponSwapGun);
    EIC->BindAction(InputConfig->IA_WeaponSwapSword,  ETriggerEvent::Started,   this, &APlayerCharacter::Input_WeaponSwapSword);
    EIC->BindAction(InputConfig->IA_WeaponSwapScroll, ETriggerEvent::Triggered, this, &APlayerCharacter::Input_WeaponSwapScroll);
}

// ─── Input handlers ────────────────────────────────────────────────────────

void APlayerCharacter::Input_Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    CurrentMoveInput = Axis;

    Glissando->OnMoveInput(Axis);
    Dash->SetMoveInput(Axis);

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
    const FVector2D Axis        = Value.Get<FVector2D>();
    const float     Sensitivity = GetLookSensitivityMultiplier();
    AddControllerYawInput(Axis.X * Sensitivity);
    AddControllerPitchInput(Axis.Y * Sensitivity);
}

void APlayerCharacter::Input_JumpStarted()   { JumpComp->TryJump(); }
void APlayerCharacter::Input_JumpCompleted() { StopJumping(); }

void APlayerCharacter::Input_SlideStarted()
{
    Glissando->OnMoveInput(CurrentMoveInput);
    if (Glissando->CanGlissando()) Glissando->StartGlissando();
}

void APlayerCharacter::Input_SlideCompleted()       { Glissando->EndGlissando(); }
void APlayerCharacter::Input_SlamStarted()          { Slam->TrySlam(); }
void APlayerCharacter::Input_AttackStarted()         { Gun->TryFire(); }
void APlayerCharacter::Input_WeaponSkillStarted()    { Focus->StartFocus(); }
void APlayerCharacter::Input_WeaponSkillCompleted()  { Focus->EndFocus(); }
void APlayerCharacter::Input_WeaponSwapGun()         { WeaponSwap->SwapToGun(); }
void APlayerCharacter::Input_WeaponSwapSword()       { WeaponSwap->SwapToSword(); }

void APlayerCharacter::Input_DashStarted()
{
    if (Slam->IsSlamming()) Slam->CancelSlam();
    Dash->SetMoveInput(CurrentMoveInput);
    Dash->TryDash();
}

void APlayerCharacter::Input_WeaponSwapScroll(const FInputActionValue& Value)
{
    WeaponSwap->SwapScroll(Value.Get<float>());
}

// ─── Overrides ─────────────────────────────────────────────────────────────

void APlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    WallJump->ResetWallJumps();
}

float APlayerCharacter::GetLookSensitivityMultiplier() const
{
    if (Focus && Focus->IsFocusing())
        return Focus->GetFocusSensitivity();
    return 1.f;
}