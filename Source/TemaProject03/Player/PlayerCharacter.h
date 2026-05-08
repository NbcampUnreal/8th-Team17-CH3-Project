#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CharacterDataStruct.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class TEMAPROJECT03_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float CharacterAttack = 50.f;

    // 무기 클래스 (블루프린트 넣을 용도)
    UPROPERTY(EditAnywhere)
    class TSubclassOf<class AWeaponBase> WeaponClass;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<class AWeaponBase> RifleClass;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<class AWeaponBase> ShotgunClass;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<class AWeaponBase> PistolClass;

    // 현재 무기
    UPROPERTY()
    class AWeaponBase* CurrentWeapon;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* FireAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* ReloadAction;


protected:
    // 카메라 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* CameraComp;

    // 입력 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* DashAction;

    // Enhanced Input
    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;


    // 데이터 테이블 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    class UDataTable* StatTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    FName CharacterRowName;

    // 이동 속도 (데이터 테이블 로드 시 덮어씌워짐)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float NormalSpeed = 1100.0f;

    // 대시 중 이동 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float DashSpeed = 3000.f;

    // 대시 지속 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float DashDuration = 0.2f;

    // 대시 쿨타임 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
    float DashCooldown = 2.0f;

    // 애님 몽타주
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* DashMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* FireMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* ReloadMontage;


    bool bIsDashing = false;       // 대시 상태 체크
    bool bIsDashOnCooldown = false; // 쿨타임 상태 체크

    FVector DashDirection;           // 대시 방향
    float OriginalMaxWalkSpeed;      // 원래 걷기 속도 저장
    float OriginalMaxAcceleration;   // 원래 가속도 저장

    FTimerHandle DashTimerHandle;         // 대시 종료용 타이머 핸들
    FTimerHandle DashCooldownTimerHandle; // 쿨타임 종료용 타이머 핸들

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MinViewPitch = -53.5f;     // 카메라 최소 각도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MaxViewPitch = 80.0f;     // 카메라 최대 각도


    // 내부 스탯 저장 변수
    float CurrentHealth;
    float AttackDamage;
    float Defense;

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Dash(const FInputActionValue& Value);
    void StopDash();
    void ResetDashCooldown();
    void StartReload();
    void StartFire();
};
