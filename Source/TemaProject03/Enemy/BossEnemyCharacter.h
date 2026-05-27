// BossEnemyCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "BossEnemyCharacter.generated.h"

// 보스 전용 상태값
UENUM(BlueprintType)
enum class EBossState : uint8
{
    Idle,    // 대기
    Intro,   // 등장 연출
    Chase,   // 플레이어 추적
    Attack,  // 기본 공격
    Dash,    // 중거리 돌진 패턴
    Hit,     // 피격
    Dead     // 사망
};

UCLASS()
class TEMAPROJECT03_API ABossEnemyCharacter : public AEnemyCharacter
{
    GENERATED_BODY()

protected:
    virtual void UpdateEnemyState() override;
    virtual void HandleEnemyState(float DeltaTime) override;
    virtual void TryAttack() override;
    virtual void OnHitEnd() override;
    virtual void ApplyDamage(float DamageAmount) override;

    // =========================
    // Boss State
    // =========================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    EBossState BossState = EBossState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Combat")
    bool bIsUsingPattern = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    bool bIsIntroPlaying = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    bool bBossKillCounted = false;

    // =========================
    // Dash Pattern
    // =========================

    // 대쉬 이동 속도
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    float DashSpeed = 1200.f;

    // 대쉬 지속 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    float DashDuration = 0.6f;

    // 대쉬 쿨타임
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    float DashCooldown = 5.0f;

    // 대쉬 사용 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    float DashMinRange = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    float DashMaxRange = 1100.f;

    // 대쉬 충돌 데미지
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    float DashDamage = 20.0f;

    // 대쉬 충돌 시 플레이어를 밀어내는 힘
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    float DashKnockbackPower = 1200.0f;

    // 대쉬 충돌 시 위로 띄우는 힘
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    float DashKnockbackUpPower = 300.0f;

    // 대쉬 중 이미 플레이어를 맞췄는지 확인
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    bool bDashHitPlayer = false;

    // 현재 대쉬 사용 가능 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Dash")
    bool bCanDash = true;

    FTimerHandle DashTimerHandle;
    FTimerHandle DashCooldownTimerHandle;

    FVector DashDirection;

    void StartDash();
    void EndDash();
    void ResetDash();

    // 대쉬 중 플레이어 충돌 체크
    void CheckDashHit();

    // 패턴/공격 후 다음 패턴까지 대기 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Combat")
    float PatternDelay = 1.5f;

    // 현재 보스 패턴 사용 가능 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Combat")
    bool bCanUsePattern = true;

    FTimerHandle PatternDelayTimerHandle;

    void ResetPatternDelay();

public:
    virtual void DestroyEnemy() override;
    virtual void OnAttackEnd() override;

    UFUNCTION(BlueprintCallable)
    void OnBossIntroEnd();
};
