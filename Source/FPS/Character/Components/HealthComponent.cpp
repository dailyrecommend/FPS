#include "Character/Components/HealthComponent.h"
#include "Misc/App.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

void UHealthComponent::TakeDamage(float Amount)
{
    if (bIsDead || Amount <= 0.f) return;
    if (InvincibleTimer > 0.f) return;

    CurrentHealth = FMath::Max(0.f, CurrentHealth - Amount);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.f)
    {
        bIsDead = true;
        OnDeath.Broadcast();
        return;
    }

    if (InvincibilityDuration > 0.f)
        InvincibleTimer = InvincibilityDuration;
}

void UHealthComponent::Heal(float Amount)
{
    if (bIsDead || Amount <= 0.f) return;

    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UHealthComponent::GetHealthPercent() const
{
    if (MaxHealth <= 0.f) return 0.f;
    return CurrentHealth / MaxHealth;
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (InvincibleTimer > 0.f)
        InvincibleTimer = FMath::Max(0.f, InvincibleTimer - FApp::GetDeltaTime());
}
