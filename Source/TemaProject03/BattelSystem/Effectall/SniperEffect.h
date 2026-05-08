#pragma once

#include "CoreMinimal.h"
#include "WeaponEffectBase.h"
#include "SniperEffect.generated.h"

UCLASS(Blueprintable)
class TEMAPROJECT03_API USniperEffect : public UWeaponEffectBase
{
    GENERATED_BODY()

public:

    virtual float ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo) override;
};
