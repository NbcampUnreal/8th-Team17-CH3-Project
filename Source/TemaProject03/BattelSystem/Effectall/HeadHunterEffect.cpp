#include "HeadHunterEffect.h"

float UHeadHunterEffect::ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo)
{
    if (bNextAttackBoost)
    {
        Damage *= 1.15f;

        bNextAttackBoost = false;

        UE_LOG(LogTemp, Warning, TEXT("HeadHunter Boost"));
    }

    return Damage;
}

void UHeadHunterEffect::OnHeadShot()
{
    bNextAttackBoost = true;

    UE_LOG(LogTemp, Warning, TEXT("Next Attack Boost Ready"));
}
