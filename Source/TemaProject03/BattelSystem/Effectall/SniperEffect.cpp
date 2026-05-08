#include "SniperEffect.h"

float USniperEffect::ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo)
{
    if (Distance >= 1300.f)
    {
        Damage *= 1.25f;
    }

    return Damage;
}
