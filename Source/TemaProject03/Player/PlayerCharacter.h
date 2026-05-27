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
class AWeaponBase;

UCLASS()
class TEMAPROJECT03_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

public:
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Stat")
    void RestoreHealthByPercent(float Percent);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    float UpgradeAmount = 10.0f;

    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void UpgradeMaxHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void UpgradeAttack(float Amount);

    bool GetRPGMuzzleTransform(FTransform& OutMuzzleTransform) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float CharacterAttack = 50.f;

    // 무기 클래스 (블루프린트 넣을 용도)
    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> WeaponClass;

    // 현재 무기
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AWeaponBase* CurrentWeapon = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    EWeaponType CurrentWeaponType = EWeaponType::None;

    // 1번 / 2번 / 3번  / 4번 키로 장착할 무기 클래스
    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> SniperClass;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> RifleClass;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> ShotgunClass;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> PistolClass;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* SniperAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* RifleAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* ShotgunAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* PistolAction;

    // 무기 교체 시 탄약을 기억하기 위한 저장소
    UPROPERTY()
    TMap<TSubclassOf<AWeaponBase>, int32> SavedAmmoMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName WeaponAttachSocketName = TEXT("hand_rSocket");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AWeaponPickup> WeaponPickupClass;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* FireAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* ReloadAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* SkillAction;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DashSpeed = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DashDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DashCooldown = 1.0f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<UWeaponEffectBase>> EffectClasses;

    UPROPERTY()
    UWeaponEffectBase* CurrentEffect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    class USoundBase* HitSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    class UParticleSystem* HitEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    TSubclassOf<class UCameraShakeBase> HitCameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    float HitEffectForwardOffset = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback|Damage")
    float HitEffectZOffset = 40.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Death")
    bool bIsDead = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
    float DeathFadeDelay = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
    float DeathFadeDuration = 3.0f;

    void Die();
    void StartDeathFade();

private:
    AActor* EquippedRPGActor = nullptr;
    FTimerHandle RPGUnequipTimerHandle;
    FTimerHandle DeathFadeTimerHandle;

    bool EquipRPG();
    void UnequipRPG();

    UPROPERTY()
    AWeaponPickup* NearbyWeaponPickup = nullptr;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* CameraComp;

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

    void SetNearbyWeaponPickup(AWeaponPickup* NewPickup);
    void Interact();
    void EquipWeapon();
    void EquipWeapon(TSubclassOf<AWeaponBase> NewWeaponClass);
    void DropCurrentWeapon();

    void EquipSniper();
    void EquipRifle();
    void EquipShotgun();
    void EquipPistol();

    void ApplyWeaponRecoil(float RecoilPitch, float RecoilYaw);
    void PlayArmsMontage(UAnimMontage* MontageToPlay);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* DashMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* FireMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* ReloadMontage;

    bool bIsDashing = false;
    bool bIsDashOnCooldown = false;

    FVector DashDirection;
    float OriginalMaxWalkSpeed;
    float OriginalMaxAcceleration;

    FTimerHandle DashTimerHandle;
    FTimerHandle DashCooldownTimerHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MinViewPitch = -20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MaxViewPitch = 60.0f;

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
