#include "Character/PlayerCharacter.h"
#include "Character/PlayerInputRouter.h"

#include "Movement/Registry/AbilityRegistry.h"
#include "Movement/Abilities/Jump/JumpAbility.h"
#include "Movement/Abilities/Dash/DashAbility.h"
#include "Movement/Abilities/Slam/SlamAbility.h"
#include "Movement/Abilities/WallJump/WallJumpAbility.h"
#include "Movement/Abilities/Slide/SlideAbility.h"

#include "Weapons/Registry/WeaponRegistry.h"
#include "Weapons/Implementations/Gun/GunWeapon.h"
#include "Weapons/Implementations/Sword/SwordWeapon.h"
#include "Weapons/Implementations/GunSkill/GunSkill.h"
#include "Weapons/Implementations/SwordSkill/SwordSkill.h"

#include "Presentation/Components/AnimationPlayerComponent.h"
#include "Presentation/Components/CameraEffectsComponent.h"

#include "Input/PlayerCharacterInputConfig.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(GetCapsuleComponent());
    Camera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
    Camera->bUsePawnControlRotation = true;

    ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
    ArmsMesh->SetupAttachment(Camera);
    ArmsMesh->SetOnlyOwnerSee(true);
    ArmsMesh->bCastDynamicShadow = false;
    ArmsMesh->CastShadow         = false;

    InputRouter     = CreateDefaultSubobject<UPlayerInputRouter>(TEXT("InputRouter"));

    AbilityRegistry = CreateDefaultSubobject<UAbilityRegistry> (TEXT("AbilityRegistry"));
    JumpAbility     = CreateDefaultSubobject<UJumpAbility>     (TEXT("JumpAbility"));
    DashAbility     = CreateDefaultSubobject<UDashAbility>     (TEXT("DashAbility"));
    SlamAbility     = CreateDefaultSubobject<USlamAbility>     (TEXT("SlamAbility"));
    WallJumpAbility = CreateDefaultSubobject<UWallJumpAbility> (TEXT("WallJumpAbility"));
    SlideAbility    = CreateDefaultSubobject<USlideAbility>    (TEXT("SlideAbility"));

    WeaponRegistry  = CreateDefaultSubobject<UWeaponRegistry>  (TEXT("WeaponRegistry"));
    GunWeapon       = CreateDefaultSubobject<UGunWeapon>       (TEXT("GunWeapon"));
    SwordWeapon     = CreateDefaultSubobject<USwordWeapon>     (TEXT("SwordWeapon"));
    GunSkill        = CreateDefaultSubobject<UGunSkill>        (TEXT("GunSkill"));
    SwordSkill      = CreateDefaultSubobject<USwordSkill>      (TEXT("SwordSkill"));

    AnimationPlayer = CreateDefaultSubobject<UAnimationPlayerComponent>(TEXT("AnimationPlayer"));
    CameraEffects   = CreateDefaultSubobject<UCameraEffectsComponent>  (TEXT("CameraEffects"));
}

void APlayerCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    WirePresentation();
    InjectAndRegisterAbilities();
    InjectAndRegisterWeapons();

    if (InputRouter)
    {
        InputRouter->InjectDependencies(this, AbilityRegistry, WeaponRegistry);
        InputRouter->SetInputConfig(InputConfig);
    }
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    RegisterInputMappingContext();

    if (InputRouter)
        InputRouter->BindInputActions(PlayerInputComponent);
}

void APlayerCharacter::RegisterInputMappingContext()
{
    if (!InputConfig) return;

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    UEnhancedInputLocalPlayerSubsystem* InputSys =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!InputSys) return;

    if (InputConfig->IMC_KeyboardMouse)
        InputSys->AddMappingContext(InputConfig->IMC_KeyboardMouse, 0);

    if (InputConfig->IMC_Gamepad)
        InputSys->AddMappingContext(InputConfig->IMC_Gamepad, 0);
}

void APlayerCharacter::WirePresentation()
{
    if (AnimationPlayer && ArmsMesh)
        AnimationPlayer->InjectMesh(ArmsMesh);

    if (CameraEffects && Camera)
        CameraEffects->InjectDependencies(Camera, BaseEyeHeight);
}

void APlayerCharacter::InjectAndRegisterAbilities()
{
    if (!AbilityRegistry) return;

    if (JumpAbility)
    {
        JumpAbility->InjectDependencies(this);
        JumpAbility->AttachAnimationPlayer(AnimationPlayer);
    }
    if (DashAbility)
    {
        DashAbility->InjectDependencies(this);
        DashAbility->AttachAnimationPlayer(AnimationPlayer);
    }
    if (SlamAbility)
    {
        SlamAbility->InjectDependencies(this);
        SlamAbility->AttachAnimationPlayer(AnimationPlayer);
    }
    if (WallJumpAbility)
    {
        WallJumpAbility->InjectDependencies(this);
        WallJumpAbility->AttachAnimationPlayer(AnimationPlayer);
    }
    if (SlideAbility)
    {
        SlideAbility->InjectDependencies(this);
        SlideAbility->AttachCameraEffects(CameraEffects);
        SlideAbility->AttachAnimationPlayer(AnimationPlayer);
    }

    AbilityRegistry->Register(JumpAbility);
    AbilityRegistry->Register(DashAbility);
    AbilityRegistry->Register(SlamAbility);
    AbilityRegistry->Register(WallJumpAbility);
    AbilityRegistry->Register(SlideAbility);
}

void APlayerCharacter::InjectAndRegisterWeapons()
{
    if (!WeaponRegistry) return;

    if (GunWeapon)
    {
        GunWeapon->InjectDependencies(this, Camera);
        GunWeapon->AttachAnimationPlayer(AnimationPlayer);
    }

    if (SwordWeapon)
    {
        SwordWeapon->InjectDependencies(this, Camera);
        SwordWeapon->AttachAnimationPlayer(AnimationPlayer);
        SwordWeapon->SetupHitbox();
    }

    if (GunSkill && GunWeapon)
    {
        GunSkill->InjectDependencies(this, Camera);
        GunSkill->AttachAnimationPlayer(AnimationPlayer);
        GunSkill->AttachCameraEffects(CameraEffects);
        GunSkill->AttachGun(GunWeapon);
        GunWeapon->AttachSkill(GunSkill);
    }

    if (SwordSkill && SwordWeapon)
    {
        SwordSkill->InjectDependencies(this, Camera);
        SwordSkill->AttachAnimationPlayer(AnimationPlayer);
        SwordSkill->AttachSword(SwordWeapon);
        SwordWeapon->AttachSkill(SwordSkill);
    }

    WeaponRegistry->Register(GunWeapon);
    WeaponRegistry->Register(SwordWeapon);
}