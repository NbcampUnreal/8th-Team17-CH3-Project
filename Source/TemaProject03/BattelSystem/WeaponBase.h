// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponData.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Effectall/WeaponEffectBase.h"
#include "Effectall/HeadHunterEffect.h"
#include "Effectall/ConfidenceEffect.h"
#include "Components/SceneComponent.h"
#include "WeaponBase.generated.h"

class ABazookaProjectile;

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

    int32 MaxAmmo;

    bool bIsReloading = false;

    FTimerHandle ReloadTimerHandle;

    bool bCanFire = true;

    FTimerHandle FireRateTimerHandle;

    void ResetFire();

    // 무기 기준점
    UPROPERTY(VisibleAnywhere)
    USceneComponent* WeaponRoot;

    // 메쉬 추가
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere)
    UNiagaraSystem* MuzzleFlash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<ABazookaProjectile> BazookaProjectileClass;

    bool bIsChargingBow = false;

    FTimerHandle BowChargeTimerHandle;

    float BowChargeStartTime = 0.0f;
    float BowChargeTime = 1.0f;

public:
    void Fire();
    void Reload();
    void FinishReload();
    int32 GetCurrentAmmo() const;
    int32 GetMaxAmmo() const;

    void StartFire();
    void StopFire();
    void FireShotgun();
    void FireBow();
    void FireBazooka();

    // WeaponData에 있는 카메라 부착값 적용
    void ApplyWeaponAttachTransform();

    // 카메라에 붙은 총기 자체를 움직이는 발사 연출
    void PlayFireFeedback();

    // 발사 연출 후 원래 위치로 복구
    void ResetWeaponFeedback();

    FTimerHandle AutoFireTimerHandle;

    // 무기의 기본 위치/회전 저장
    FVector BaseRelativeLocation;
    FRotator BaseRelativeRotation;

    FTimerHandle FireFeedbackTimerHandle;
};
