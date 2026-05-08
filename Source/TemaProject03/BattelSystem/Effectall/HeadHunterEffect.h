#pragma once

#include "CoreMinimal.h"
#include "WeaponEffectBase.h"
#include "HeadHunterEffect.generated.h"

UCLASS()
class TEMAPROJECT03_API UHeadHunterEffect : public UWeaponEffectBase
{
    GENERATED_BODY()

public:

    virtual float ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo) override;

    void OnHeadShot();

private:

    bool bNextAttackBoost = false;
};
