// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponEffectBase.h"
#include "PlusMagazineEffect.generated.h"

/**
 * 
 */
UCLASS()
class TEMAPROJECT03_API UPlusMagazineEffect : public UWeaponEffectBase
{
	GENERATED_BODY()

public:

    virtual int32 ModifyMagazineSize(int32 MagazineSize) override;
};
