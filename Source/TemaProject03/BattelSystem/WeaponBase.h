// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponData.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "WeaponBase.generated.h"

UCLASS()
class TEMAPROJECT03_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AWeaponBase();

protected:
    virtual void BeginPlay() override;

public:
    // DataTable 연결
    UPROPERTY(EditAnywhere)
    UDataTable* WeaponTable;

    // 어떤 무기인지
    UPROPERTY(EditAnywhere)
    FName WeaponRowName;

    // 실제 데이터
    FWeaponData WeaponData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentAmmo;

    bool bIsReloading = false;

    FTimerHandle ReloadTimerHandle;

    // 메쉬 추가
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere)
    UParticleSystem* MuzzleFlash;



public:
    void Fire();
    void Reload();
    void FinishReload();
    int32 GetCurrentAmmo() const;
};
