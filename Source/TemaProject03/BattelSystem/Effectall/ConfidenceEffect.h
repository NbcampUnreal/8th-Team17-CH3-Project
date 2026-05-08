// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponEffectBase.h"
#include "ConfidenceEffect.generated.h"

/**
 * 
 */
UCLASS()
class TEMAPROJECT03_API UConfidenceEffect : public UWeaponEffectBase
{
	GENERATED_BODY()

public:

    virtual float ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo) override;

    void OnHeadShot();

    void OnBodyShot();

private:

    float DamageBonus = 0.f;
};
