// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponEffectBase.h"
#include "ProximityEffect.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class TEMAPROJECT03_API UProximityEffect : public UWeaponEffectBase
{
    GENERATED_BODY()

public:
    virtual float ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo) override;
};
//if (CurrentAmmo >= MaxAmmo / 2)
//{
//    Damage *= 1.09f;
//}
