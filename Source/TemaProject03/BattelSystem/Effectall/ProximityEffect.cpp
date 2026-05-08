// Fill out your copyright notice in the Description page of Project Settings.


#include "ProximityEffect.h"

float UProximityEffect::ModifyDamage(float Damage, float Distance, int32 CurrentAmmo, int32 MaxAmmo)
{
    if (Distance <= 200.f)
    {
        Damage *= 1.5f;
    }

    return Damage;
}
