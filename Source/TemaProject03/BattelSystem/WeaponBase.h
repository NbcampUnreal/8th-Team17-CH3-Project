#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponData.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Effectall/WeaponEffectBase.h"
#include "Effectall/HeadHunterEffect.h"
#include "Effectall/ConfidenceEffect.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
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
    UPROPERTY(EditAnywhere, Category = "Weapon|Data")
    UDataTable* WeaponTable;

    // 어떤 무기인지
    UPROPERTY(EditAnywhere, Category = "Weapon|Data")
    FName WeaponRowName;

    // 실제 데이터
    FWeaponData WeaponData;

    // 무기 타입 - 팔 ABP에서 Idle 자세 선택용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
    EWeaponType WeaponType = EWeaponType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
    int32 CurrentAmmo = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
    int32 MaxAmmo = 0;

    bool bIsReloading = false;
    FTimerHandle ReloadTimerHandle;

    bool bCanFire = true;
    FTimerHandle FireRateTimerHandle;
    FTimerHandle AutoFireTimerHandle;

    void ResetFire();

    // 무기 기준점
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    USceneComponent* WeaponRoot;

    // Static Mesh 총기용
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    UStaticMeshComponent* Mesh;

    // Skeletal Mesh 총기용
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    USkeletalMeshComponent* SkeletalMesh;

    // 총기 자체 발사 애니메이션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
    UAnimSequence* FireWeaponAnimation;

    // 총기 자체 장전 애니메이션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
    UAnimSequence* ReloadWeaponAnimation;

    // 팔 발사 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
    UAnimMontage* ArmsFireMontage;

    // 팔 장전 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
    UAnimMontage* ArmsReloadMontage;

    // 발사 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|FX")
    USoundBase* FireSound;

    // 장전 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|FX")
    USoundBase* ReloadSound;

    // 발사 이펙트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|FX")
    UParticleSystem* FireEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<ABazookaProjectile> BazookaProjectileClass;

    // 효과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effect")
    TArray<TSubclassOf<UWeaponEffectBase>> EffectClasses;

    UPROPERTY()
    UWeaponEffectBase* CurrentEffect = nullptr;

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

    // 소켓 장착 후 상대 위치 초기화
    void ApplyWeaponAttachTransform();

    UMeshComponent* GetActiveWeaponMesh() const;
    FVector GetMuzzleLocation() const;
    FRotator GetMuzzleRotation() const;
};
