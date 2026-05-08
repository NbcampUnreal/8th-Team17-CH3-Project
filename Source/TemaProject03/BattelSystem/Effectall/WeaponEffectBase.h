#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Object.h"
#include "WeaponEffectBase.generated.h"

UCLASS(Blueprintable)
class TEMAPROJECT03_API UWeaponEffectBase : public UObject
{
    GENERATED_BODY()

public:

    virtual float ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo);

    virtual int32 ModifyMagazineSize(int32 MagazineSize);
};
