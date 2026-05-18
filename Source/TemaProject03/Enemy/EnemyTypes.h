// EnemyTypes.h

#pragma once

#include "CoreMinimal.h"
#include "EnemyTypes.generated.h"


// 몬스터 상태 Emum 추가
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Chase UMETA(DisplayName = "Chase"),
    Attack UMETA(DisplayName = "Attack"),
    Dead UMETA(DisplayName = "Dead")
};


// 공격 타입 추가 (원거리, 근거리)
UENUM(BlueprintType)
enum class EEnemyAttackType : uint8
{
    Melee UMETA(DisplayName = "Melee"),
    Ranged UMETA(DisplayName = "Ranged"),
    Both UMETA(DisplayName = "Both")
};

// 회피 방식 추가
UENUM(BlueprintType)
enum class EEnemyEvadePhase : uint8
{
    None UMETA(DisplayName = "None"),
    Back UMETA(DisplayName = "Back"),
    Left UMETA(DisplayName = "Left"),
    Right UMETA(DisplayName = "Right")
};
