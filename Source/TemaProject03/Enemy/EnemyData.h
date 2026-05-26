// EnemyData.h

#pragma once

#include "CoreMinimal.h"
#include "EnemyTypes.h"
#include "Engine/DataTable.h"
#include "EnemyData.generated.h"

USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
    GENERATED_BODY()

public:

    // =========================
    // Basic Stat
    // =========================

    // 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 100.0f;

    // 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackPower = 30.0f;

    // 방어력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Defense = 10.0f;

    // 이동 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoveSpeed = 400.0f;

    // =========================
    // Detect / Attack
    // =========================

    // 감지 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectRange = 800.0f;

    // 공격 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 600.0f;

    // 공격 쿨타임
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackCooldown = 2.0f;

    // 공격 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnemyAttackType AttackType = EEnemyAttackType::Melee;

    // =========================
    // Jump Attack
    // =========================

    // 점프 공격 사용 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseJumpAttack = false;

    // 점프 공격 최소 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpAttackMinRange = 250.0f;

    // 점프 공격 최대 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpAttackMaxRange = 900.0f;

    // 점프 공격 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpAttackDamage = 50.0f;

    // 점프 공격 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpAttackRadius = 250.0f;

    // 점프 공격 쿨타임
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpAttackCooldown = 5.0f;

    // 점프 공격 착지 보정 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpAttackLandingOffset = 100.0f;

    // 점프 공격 이동 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpAttackMoveTime = 0.5f;

    // =========================
    // Patrol
    // =========================

    // 순찰 반경
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatrolRadius = 500.0f;

    // 순찰 대기 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatrolWaitTime = 2.0f;

    // =========================
    // Sense
    // =========================

    // 시야 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SightRadius = 800.0f;

    // 시야각
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VisionAngle = 22.5f;

    // 감지 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SensingInterval = 0.2f;
};
