#include "ConfidenceEffect.h"

float UConfidenceEffect::ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo)
{
    Damage *= (1.f + DamageBonus);

    return Damage;
}

void UConfidenceEffect::OnHeadShot()
{
    DamageBonus += 0.05f;

    DamageBonus = FMath::Clamp(DamageBonus, -0.5f, 0.6f);

    UE_LOG(LogTemp, Warning, TEXT("Bonus Increased: %f"), DamageBonus);
}

void UConfidenceEffect::OnBodyShot()
{
    DamageBonus -= 0.09f;

    DamageBonus = FMath::Clamp(DamageBonus, -0.5f, 0.6f);

    UE_LOG(LogTemp, Warning, TEXT("Bonus Decreased: %f"), DamageBonus);
}
