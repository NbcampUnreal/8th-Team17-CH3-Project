#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "CharacterDataStruct.h"
#include "TemaProject03/BattelSystem/Effectall/WeaponEffectBase.h"
#include "TemaProject03/Enemy/EnemySpawner.h"
#include "TemaProject03/BattelSystem/WeaponData.h"

#include "PlayerCharacter.generated.h"

class AWeaponPickup;

UCLASS()
class TEMAPROJECT03_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

public:
    void ApplyDamage(float DamageAmount);

    // RPG 탄두 스폰 위치를 스킬에서 가져가기 위한 함수
    bool GetRPGMuzzleTransform(FTransform& OutMuzzleTransform) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float CharacterAttack = 50.f;

    // 무기 클래스 (블루프린트 넣을 용도)
    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<class AWeaponBase> WeaponClass;

    // 현재 무기
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    class AWeaponBase* CurrentWeapon = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    EWeaponType CurrentWeaponType = EWeaponType::None;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> RifleClass;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> ShotgunClass;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> PistolClass;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* RifleAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* ShotgunAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* PistolAction;

    UPROPERTY()
    TMap<TSubclassOf<AWeaponBase>, int32> SavedAmmoMap;

    // 손에 무기를 붙일 소켓 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName WeaponAttachSocketName = TEXT("hand_rSocket");

    // 바닥에 버릴 무기 Pickup 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AWeaponPickup> WeaponPickupClass;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* FireAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* ReloadAction;

    // F키 상호작용 입력 액션
    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* InteractAction;

    // 스킬 입력 액션
    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* SkillAction;

    // 스킬 컴포넌트 포인터
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
    class USkillComponent* SkillComp;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* DashAction;

    UPROPERTY(EditAnywhere, Category = "Stat")
    class UDataTable* StatTable;

    UPROPERTY(EditAnywhere, Category = "Stat")
    FName CharacterRowName;

    // 대시 스펙 설정 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DashSpeed = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DashDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DashCooldown = 1.0f;

    // RPG 장착 설정 관련 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RPG")
    TSubclassOf<AActor> RPGWeaponActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RPG")
    float RPGEquipDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RPG")
    FVector RPGAttachLocation = FVector(25.f, 15.f, -15.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RPG")
    FRotator RPGAttachRotation = FRotator(0.f, 0.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|RPG")
    FVector RPGAttachScale = FVector(1.0f, 1.0f, 1.0f);

    // 효과
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<UWeaponEffectBase>> EffectClasses;

    UPROPERTY()
    UWeaponEffectBase* CurrentEffect = nullptr;

    // 플레이어가 피해를 입었을 때 재생할 피격 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    class USoundBase* HitSound;

    // 플레이어가 피해를 입었을 때 생성할 피격 파티클 이펙트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    class UParticleSystem* HitEffect;

    // 플레이어가 피해를 입었을 때 재생할 카메라 흔들림 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    TSubclassOf<class UCameraShakeBase> HitCameraShake;

    // 피격 이펙트를 캐릭터 기준 앞쪽으로 얼마나 띄워서 생성할지 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    float HitEffectForwardOffset = 30.0f;

    // 피격 이펙트를 캐릭터 기준 위쪽으로 얼마나 올려서 생성할지 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    float HitEffectZOffset = 40.0f;

private:
    AActor* EquippedRPGActor = nullptr;
    FTimerHandle RPGUnequipTimerHandle;

    bool EquipRPG();
    void UnequipRPG();

    UPROPERTY()
    AWeaponPickup* NearbyWeaponPickup = nullptr;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* CameraComp;

    // 1인칭 팔 메시
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    class USkeletalMeshComponent* FirstPersonArmsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float AttackDamage = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float Defense = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float NormalSpeed = 600.f;

    void UseSkillInput();

    void StartFire();
    void StopFire();

    // 바닥 총 감지/교체
    void SetNearbyWeaponPickup(AWeaponPickup* NewPickup);
    void Interact();
    void EquipWeapon();
    void EquipWeapon(TSubclassOf<class AWeaponBase> NewWeaponClass);
    void DropCurrentWeapon();

    void EquipRifle();
    void EquipShotgun();
    void EquipPistol();
    void ChangeWeapon(TSubclassOf<AWeaponBase> NewWeaponClass);

    // 무기 발사 시 카메라 반동 처리
    void ApplyWeaponRecoil(float RecoilPitch, float RecoilYaw);

    void PlayArmsMontage(UAnimMontage* MontageToPlay);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* DashMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* FireMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* ReloadMontage;

    bool bIsDashing = false;           // 대시 상태 체크
    bool bIsDashOnCooldown = false; // 쿨타임 상태 체크

    FVector DashDirection;            // 대시 방향
    float OriginalMaxWalkSpeed;      // 원래 걷기 속도 저장
    float OriginalMaxAcceleration;   // 원래 가속도 저장

    FTimerHandle DashTimerHandle;         // 대시 종료용 타이머 핸들
    FTimerHandle DashCooldownTimerHandle; // 쿨타임 종료용 타이머 핸들

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MinViewPitch = -20.0f;     // 카메라 최소 각도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MaxViewPitch = 60.0f;     // 카메라 최대 각도

    float GetCurrentHealth() const { return CurrentHealth; }
    float GetMaxHealth() const { return MaxHealth; }

    float GetOriginalSkillCooldown() const;
    class USkillComponent* GetSkillComponent() const { return SkillComp; }
    float GetDashCooldown() const { return DashCooldown; }
    float GetRemainingCooldown() const;

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Dash(const FInputActionValue& Value);
    void StopDash();
    void ResetDashCooldown();
    void StartReload();
};
