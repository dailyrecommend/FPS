#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Weapons/Data/WeaponChangedEvent.h"
#include "PlayerHUDWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;
class APlayerCharacter;

UCLASS()
class FPS_API UPlayerHUDWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    void InitBindings(APlayerCharacter* Player);

    UFUNCTION() void OnWeaponChanged(const FWeaponChangedEvent& Event);
    UFUNCTION() void OnHealthChanged(float Current, float Max);
    UFUNCTION() void OnDashChargesChanged(int32 Current, int32 Max);

    void UpdateDashBars(int32 Current);

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    float DashBarCurrent[3] = { 1.f, 1.f, 1.f };
    float DashBarTarget [3] = { 1.f, 1.f, 1.f };

    UPROPERTY(EditDefaultsOnly, Category = "HUD|Dash") float DashBarInterpSpeed = 6.f;

    // ── 디자이너에서 이름 동일하게 배치 ──

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> WeaponImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> HealthText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> DashBar1;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> DashBar2;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> DashBar3;

    // ── BP 디테일 패널에서 텍스처 할당 ──

    UPROPERTY(EditDefaultsOnly, Category = "HUD|Weapon")
    TObjectPtr<UTexture2D> GunTexture;

    UPROPERTY(EditDefaultsOnly, Category = "HUD|Weapon")
    TObjectPtr<UTexture2D> SwordTexture;

    float CachedMaxHealth = 100.f;
};
