#include "Character/PlayerInputRouter.h"
#include "Movement/Registry/AbilityRegistry.h"
#include "Movement/Data/AbilityContext.h"
#include "Weapons/Registry/WeaponRegistry.h"
#include "Weapons/Interfaces/Weapon.h"
#include "Weapons/Interfaces/WeaponSkill.h"
#include "Input/PlayerCharacterInputConfig.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

UPlayerInputRouter::UPlayerInputRouter()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerInputRouter::InjectDependencies(ACharacter* InOwner, UAbilityRegistry* InAbilityRegistry, UWeaponRegistry* InWeaponRegistry)
{
    OwnerCharacter  = InOwner;
    AbilityRegistry = InAbilityRegistry;
    WeaponRegistry  = InWeaponRegistry;
}

void UPlayerInputRouter::BindInputActions(UInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent || !InputConfig) return;

    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!Input) return;

    if (InputConfig->IA_Move)             Input->BindAction(InputConfig->IA_Move,             ETriggerEvent::Triggered, this, &UPlayerInputRouter::Input_Move);
    if (InputConfig->IA_Move)             Input->BindAction(InputConfig->IA_Move,             ETriggerEvent::Completed, this, &UPlayerInputRouter::Input_Move);
    if (InputConfig->IA_Look)             Input->BindAction(InputConfig->IA_Look,             ETriggerEvent::Triggered, this, &UPlayerInputRouter::Input_Look);
    if (InputConfig->IA_Jump)             Input->BindAction(InputConfig->IA_Jump,             ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_JumpStarted);
    if (InputConfig->IA_Dash)             Input->BindAction(InputConfig->IA_Dash,             ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_DashStarted);
    if (InputConfig->IA_Slam)             Input->BindAction(InputConfig->IA_Slam,             ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_SlamStarted);
    if (InputConfig->IA_Slide)            Input->BindAction(InputConfig->IA_Slide,            ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_SlideStarted);
    if (InputConfig->IA_Slide)            Input->BindAction(InputConfig->IA_Slide,            ETriggerEvent::Completed, this, &UPlayerInputRouter::Input_SlideCompleted);
    if (InputConfig->IA_Attack)           Input->BindAction(InputConfig->IA_Attack,           ETriggerEvent::Triggered,   this, &UPlayerInputRouter::Input_AttackStarted);
    if (InputConfig->IA_WeaponSkill)      Input->BindAction(InputConfig->IA_WeaponSkill,      ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_WeaponSkillStarted);
    if (InputConfig->IA_WeaponSkill)      Input->BindAction(InputConfig->IA_WeaponSkill,      ETriggerEvent::Completed, this, &UPlayerInputRouter::Input_WeaponSkillCompleted);
    if (InputConfig->IA_WeaponSwapGun)    Input->BindAction(InputConfig->IA_WeaponSwapGun,    ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_WeaponSwapGun);
    if (InputConfig->IA_WeaponSwapSword)  Input->BindAction(InputConfig->IA_WeaponSwapSword,  ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_WeaponSwapSword);
    if (InputConfig->IA_WeaponSwapScroll) Input->BindAction(InputConfig->IA_WeaponSwapScroll, ETriggerEvent::Triggered, this, &UPlayerInputRouter::Input_WeaponSwapScroll);
    if (InputConfig->IA_WeaponSwapNext)   Input->BindAction(InputConfig->IA_WeaponSwapNext,   ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_WeaponSwapNext);
    if (InputConfig->IA_WeaponSwapPrev)   Input->BindAction(InputConfig->IA_WeaponSwapPrev,   ETriggerEvent::Started,   this, &UPlayerInputRouter::Input_WeaponSwapPrev);
}

FAbilityContext UPlayerInputRouter::BuildContext() const
{
    FAbilityContext Ctx;
    Ctx.MoveInput = CurrentMoveInput;

    ACharacter* Owner = OwnerCharacter.Get();
    if (!Owner) return Ctx;

    Ctx.Velocity = Owner->GetVelocity();

    if (UCharacterMovementComponent* MoveComp = Owner->GetCharacterMovement())
        Ctx.bGrounded = MoveComp->IsMovingOnGround();

    return Ctx;
}

void UPlayerInputRouter::Input_Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    CurrentMoveInput = Axis;

    ACharacter* Owner = OwnerCharacter.Get();
    if (!Owner || Axis.IsNearlyZero()) return;

    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC) return;

    const FRotator        CamRot = PC->GetControlRotation();
    const FRotationMatrix YawMatrix(FRotator(0.f, CamRot.Yaw, 0.f));
    const FVector         Forward = YawMatrix.GetUnitAxis(EAxis::X);
    const FVector         Right   = YawMatrix.GetUnitAxis(EAxis::Y);

    Owner->AddMovementInput(Forward, Axis.Y);
    Owner->AddMovementInput(Right,   Axis.X);
}

void UPlayerInputRouter::Input_Look(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    ACharacter* Owner = OwnerCharacter.Get();
    if (!Owner) return;

    Owner->AddControllerYawInput(Axis.X);
    Owner->AddControllerPitchInput(Axis.Y);
}

void UPlayerInputRouter::Input_JumpStarted()
{
    if (UAbilityRegistry* Registry = AbilityRegistry.Get())
        Registry->TryActivate(TEXT("Jump"), BuildContext());
}

void UPlayerInputRouter::Input_DashStarted()
{
    if (UAbilityRegistry* Registry = AbilityRegistry.Get())
        Registry->TryActivate(TEXT("Dash"), BuildContext());
}

void UPlayerInputRouter::Input_SlamStarted()
{
    if (UAbilityRegistry* Registry = AbilityRegistry.Get())
        Registry->TryActivate(TEXT("Slam"), BuildContext());
}

void UPlayerInputRouter::Input_SlideStarted()
{
    if (UAbilityRegistry* Registry = AbilityRegistry.Get())
        Registry->TryActivate(TEXT("Slide"), BuildContext());
}

void UPlayerInputRouter::Input_SlideCompleted()
{
    if (UAbilityRegistry* Registry = AbilityRegistry.Get())
        Registry->Cancel(TEXT("Slide"));
}

void UPlayerInputRouter::Input_AttackStarted()
{
    UWeaponRegistry* Registry = WeaponRegistry.Get();
    if (!Registry) return;

    TScriptInterface<IWeapon> Weapon = Registry->GetCurrentWeapon();
    UObject* WeaponObj = Weapon.GetObject();
    if (!WeaponObj) return;

    TScriptInterface<IWeaponSkill> Skill = IWeapon::Execute_GetSkill(WeaponObj);
    if (UObject* SkillObj = Skill.GetObject())
        if (IWeaponSkill::Execute_IsSkillActive(SkillObj)) return;

    IWeapon::Execute_TryAttack(WeaponObj);
}

void UPlayerInputRouter::Input_WeaponSkillStarted()
{
    UWeaponRegistry* Registry = WeaponRegistry.Get();
    if (!Registry) return;

    TScriptInterface<IWeapon> Weapon = Registry->GetCurrentWeapon();
    UObject* WeaponObj = Weapon.GetObject();
    if (!WeaponObj) return;

    TScriptInterface<IWeaponSkill> Skill = IWeapon::Execute_GetSkill(WeaponObj);
    if (UObject* SkillObj = Skill.GetObject())
        IWeaponSkill::Execute_StartHold(SkillObj);
}

void UPlayerInputRouter::Input_WeaponSkillCompleted()
{
    UWeaponRegistry* Registry = WeaponRegistry.Get();
    if (!Registry) return;

    TScriptInterface<IWeapon> Weapon = Registry->GetCurrentWeapon();
    UObject* WeaponObj = Weapon.GetObject();
    if (!WeaponObj) return;

    TScriptInterface<IWeaponSkill> Skill = IWeapon::Execute_GetSkill(WeaponObj);
    if (UObject* SkillObj = Skill.GetObject())
        IWeaponSkill::Execute_EndHold(SkillObj);
}

void UPlayerInputRouter::Input_WeaponSwapGun()
{
    if (UWeaponRegistry* Registry = WeaponRegistry.Get())
        Registry->SwapTo(TEXT("Gun"));
}

void UPlayerInputRouter::Input_WeaponSwapSword()
{
    if (UWeaponRegistry* Registry = WeaponRegistry.Get())
        Registry->SwapTo(TEXT("Sword"));
}

void UPlayerInputRouter::Input_WeaponSwapScroll(const FInputActionValue& Value)
{
    UWeaponRegistry* Registry = WeaponRegistry.Get();
    if (!Registry) return;

    const float Axis = Value.Get<float>();
    if (FMath::IsNearlyZero(Axis)) return;

    if (Axis > 0.f) Registry->CycleNext();
    else            Registry->CyclePrevious();
}

void UPlayerInputRouter::Input_WeaponSwapNext()
{
    if (UWeaponRegistry* Registry = WeaponRegistry.Get())
        Registry->CycleNext();
}

void UPlayerInputRouter::Input_WeaponSwapPrev()
{
    if (UWeaponRegistry* Registry = WeaponRegistry.Get())
        Registry->CyclePrevious();
}