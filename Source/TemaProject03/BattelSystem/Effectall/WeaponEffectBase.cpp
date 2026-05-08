#include "WeaponEffectBase.h"

float UWeaponEffectBase::ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo)
{
    return Damage;
}

int32 UWeaponEffectBase::ModifyMagazineSize(int32 MagazineSize)
{
    return MagazineSize;
}
