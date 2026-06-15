#include "UI/PlayerHUDWidget.h"
#include "Character/PlayerCharacter.h"
#include "Character/Components/HealthComponent.h"
#include "Movement/Abilities/Dash/DashAbility.h"
#include "Weapons/Registry/WeaponRegistry.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"

void UPlayerHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (Player)
        InitBindings(Player);
}

void UPlayerHUDWidget::InitBindings(APlayerCharacter* Player)
{
    // 무기
    if (UWeaponRegistry* Registry = Player->GetWeaponRegistry())
    {
        Registry->OnWeaponChanged.AddDynamic(this, &UPlayerHUDWidget::OnWeaponChanged);

        // 초기값
        FWeaponChangedEvent InitEvent;
        InitEvent.NewWeaponId = Registry->GetCurrentWeaponId();
        OnWeaponChanged(InitEvent);
    }

    // 체력
    if (UHealthComponent* Health = Player->GetHealth())
    {
        Health->OnHealthChanged.AddDynamic(this, &UPlayerHUDWidget::OnHealthChanged);

        // 초기값
        OnHealthChanged(Health->GetCurrentHealth(), Health->GetMaxHealth());
    }

    // 대시
    if (UDashAbility* Dash = Player->GetDashAbility())
    {
        Dash->OnDashChargesChanged.AddDynamic(this, &UPlayerHUDWidget::OnDashChargesChanged);

        // 초기값
        OnDashChargesChanged(Dash->GetDashCharges(), Dash->GetMaxDashCharges());
    }
}

void UPlayerHUDWidget::OnWeaponChanged(const FWeaponChangedEvent& Event)
{
    if (!WeaponImage) return;

    UTexture2D* Texture = nullptr;

    if (Event.NewWeaponId == TEXT("Gun"))   Texture = GunTexture;
    if (Event.NewWeaponId == TEXT("Sword")) Texture = SwordTexture;

    if (Texture)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(Texture);
        Brush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
        WeaponImage->SetBrush(Brush);
    }
}

void UPlayerHUDWidget::OnHealthChanged(float Current, float Max)
{
    CachedMaxHealth = Max;

    if (HealthBar)
        HealthBar->SetPercent(Max > 0.f ? Current / Max : 0.f);

    if (HealthText)
        HealthText->SetText(FText::FromString(
            FString::Printf(TEXT("%d / %d"),
                FMath::RoundToInt(Current),
                FMath::RoundToInt(Max))));
}

void UPlayerHUDWidget::OnDashChargesChanged(int32 Current, int32 Max)
{
    UpdateDashBars(Current);
}

void UPlayerHUDWidget::UpdateDashBars(int32 Current)
{
    DashBarTarget[0] = Current >= 1 ? 1.f : 0.f;
    DashBarTarget[1] = Current >= 2 ? 1.f : 0.f;
    DashBarTarget[2] = Current >= 3 ? 1.f : 0.f;
}

void UPlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UProgressBar* Bars[3] = { DashBar1, DashBar2, DashBar3 };
    for (int32 i = 0; i < 3; ++i)
    {
        if (!Bars[i]) continue;
        DashBarCurrent[i] = FMath::FInterpTo(DashBarCurrent[i], DashBarTarget[i], InDeltaTime, DashBarInterpSpeed);
        Bars[i]->SetPercent(DashBarCurrent[i]);
    }
}
