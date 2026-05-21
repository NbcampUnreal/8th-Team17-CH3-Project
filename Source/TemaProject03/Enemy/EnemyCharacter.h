// EnemyCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "EnemyData.h"
#include "EnemyTypes.h"
#include "TemaProject03/Player/PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "EnemyCharacter.generated.h"

class USphereComponent;
class UPawnSensingComponent;

UCLASS()
class TEMAPROJECT03_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

    public:
        AEnemyCharacter();

    protected:
        virtual void BeginPlay() override;

    public:
        virtual void Tick(float DeltaTime) override;

    protected:

        // =========================
        // Attack Setting
        // =========================

        // 근처 감지 범위
        // DetectSphere의 반경으로 사용
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
        float DetectRange = 400.0f;

        // 공격 가능 범위
        // AttackSphere의 반경으로 사용
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
        float AttackRange = 200.0f;

        // 공격 쿨타임
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
        float AttackCooldown = 2.0f;

        // 현재 Enemy 상태
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
        EEnemyState EnemyState = EEnemyState::Idle;

        // 현재 공격 가능한지 여부
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
        bool bCanAttack = true;

        // =========================
        // Pawn Sensing Setting
        // =========================

        // AI가 눈으로 볼 수 있는 거리
        // PawnSensing->SightRadius에 적용
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Sense")
        float SightRadius = 300.0f;

        // AI 시야각
        // 정면 기준으로 어느 정도 넓게 볼지 설정
        // 예: 60이면 전방 시야각으로 사용
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Sense")
        float VisionAngle = 22.5f;

        // AI가 몇 초마다 시야 감지를 검사할지 설정
        // 낮을수록 반응이 빠르지만 연산이 조금 더 많아짐
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Sense")
        float SensingInterval = 0.2f;

        // =========================
        // AI Setting
        // =========================

        // 플레이어를 바라볼 때 회전 속도
        // 값이 높을수록 더 빠르게 회전
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
        float RotationSpeed = 5.0f;

    protected:
        // =========================
        // Patrol Setting
        // =========================

        // 순찰 반경
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol")
        float PatrolRadius = 500.0f;

        // 다음 순찰까지 대기 시간
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol")
        float PatrolWaitTime = 2.0f;

        // 처음 스폰된 위치 저장
        FVector StartLocation;

        // 다음 순찰 위치로 이동할 수 있는지
        bool bCanPatrol = true;

        // 순찰 대기 타이머
        FTimerHandle PatrolTimerHandle;

        // 랜덤 순찰 이동
        void PatrolMove();

        // 순찰 가능 상태로 복구
        void ResetPatrol();

        // DataTable에서 몬스터 데이터 불러오기
        void LoadEnemyData();

        // 몬스터 기본 스탯 초기화
        // CurrentHealth = MaxHealth
        // MoveSpeed를 CharacterMovement에 적용
        void InitEnemyStat();

        // AI 시야 감지 설정 초기화
        // SightRadius, VisionAngle, SensingInterval 값을 PawnSensing에 적용
        void InitPawnSensing();

        // =========================
        // Range
        // =========================

        // 플레이어 근처 감지용 Sphere
        // 범위 안에 플레이어가 들어왔는지 확인
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Range")
        USphereComponent* DetectSphere;

        // 공격 가능 범위 Sphere
        // 플레이어가 공격 거리 안에 있는지 확인
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Range")
        USphereComponent* AttackSphere;

        // =========================
        // AI Sense
        // =========================

        // 실제 시야 감지용 컴포넌트
        // 플레이어를 "눈으로 봤는지" 판단
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Sense")
        UPawnSensingComponent* PawnSensing;

        // =========================
        // Target
        // =========================

        // 현재 감지 중인 플레이어
        // DetectSphere 또는 PawnSensing에서 확인되면 저장
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Target")
        AActor* TargetPlayer;

        // DetectSphere 안에 플레이어가 들어와 있는지 저장
        // 근처에 있는지 여부 확인용
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Target")
        bool bPlayerInDetectRange = false;

        // PawnSensing으로 플레이어를 실제로 봤는지 저장
        // 시야에 들어왔는지 여부 확인용
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Target")
        bool bCanSeePlayer = false;

        // =========================
        // Detect Overlap Function
        // =========================

        // DetectSphere 범위 안에 Actor가 들어왔을 때 호출
        // Player 태그가 있으면 bPlayerInDetectRange = true 처리
        UFUNCTION()
        void OnDetectBeginOverlap(
            UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult
        );

        // DetectSphere 범위 밖으로 Actor가 나갔을 때 호출
        // 현재 TargetPlayer가 나가면 TargetPlayer 초기화
        UFUNCTION()
        void OnDetectEndOverlap(
            UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex
        );

        // =========================
        // Pawn Sensing Function
        // =========================

        // PawnSensingComponent가 Pawn을 봤을 때 호출
        // Player 태그가 있으면 bCanSeePlayer = true 처리
        UFUNCTION()
        void OnSeePawn(APawn* SeenPawn);

        // =========================
        // Enemy AI
        // =========================

        // 감지 조건 확인
        bool CanDetectPlayer() const;

        // 플레이어 바라보기
        void LookAtTarget(float DeltaTime);

        // 플레이어 추적
        void ChaseTarget();

        // 공격 범위 안인지 확인
        bool IsTargetInAttackRange() const;

        // 공격 시도
        virtual void TryAttack();

    public:
        // 근접 공격
        UFUNCTION(BlueprintCallable)
        void PerformMeleeAttack();

        UFUNCTION(BlueprintCallable)
        virtual void OnAttackEnd();

        // 적 죽음
        UFUNCTION(BlueprintCallable)
        void DestroyEnemy();

        UFUNCTION(BlueprintCallable)
        void OnHitEnd();

    protected:
        // 공격 방식
        
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Evade")
        bool bIsEvading = false;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Evade")
        EEnemyEvadePhase EvadePhase = EEnemyEvadePhase::None;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Evade")
        float EvadeDirection = 0.0f;

        float EvadeTimer = 0.0f;
        float BackEvadeTime = 0.8f;
        float SideEvadeTime = 1.0f;

        int32 SideDirectionSign = 1;

        void StartMeleeEvade();
        void HandleMeleeEvade(float DeltaTime);
        void EndMeleeEvade();

        // 공격 쿨타임 초기화
        void ResetAttack();

        // 상태 갱신
        virtual void UpdateEnemyState();

        // 현재 상태 실행
        virtual void HandleEnemyState(float DeltaTime);

        // 상태 변경
        void SetEnemyState(EEnemyState NewState);

        FTimerHandle AttackCooldownTimerHandle;
public:
    bool bIsAttacking = false;

    FTimerHandle AttackDelayTimerHandle;

    void PerformRangedAttack();

    void ApplyDamage(float DamageAmount);

    // =========================
    // Basic Stat
    // =========================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stat")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
    float AttackPower = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
    float Defense = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
    float MoveSpeed = 400.0f;

    // =========================
    // Data Table
    // =========================

    // 사용할 Enemy 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Data")
    UDataTable* EnemyDataTable;

    // DataTable에서 사용할 Row 이름
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Data")
    FName EnemyDataRowName;

    void ApplyEnemyData();

    // 공격 타입
    // Melee = 근접 공격
    // Ranged = 원거리 공격
    // Both = 둘 다 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
    EEnemyAttackType AttackType = EEnemyAttackType::Ranged;
};
