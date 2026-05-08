#include "HabitReloadEffect.h"

float UHabitReloadEffect::ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo)
{
    if (CurrentAmmo >= MaxAmmo / 2)
    {
        Damage *= 1.09f;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Habit Reload Bonus")
        );
    }

    return Damage;
}
