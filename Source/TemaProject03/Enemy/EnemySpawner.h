// EnemySpawner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "EnemySpawner.generated.h"

// 전방 선언
class AEnemyCharacter;

UCLASS()
class TEMAPROJECT03_API AEnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnemySpawner();

protected:
    virtual void BeginPlay() override;

protected:
    // =========================
    // Spawn Setting
    // =========================

    // 일반 몬스터 클래스
    // BP_Enemy_A / BP_Enemy_B 등을 Details에서 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Class")
    TSubclassOf<AEnemyCharacter> EnemyClass;

    // 이 스포너가 점프 몬스터를 스폰하는지 여부
    // false = BP_EnemyCharacter_A / true = BP_EnemyCharacter_B
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Class")
    bool bSpawnJumpEnemy = false;

    // 보스 몬스터 클래스
    // 150킬마다 등장할 보스 BP를 Details에서 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Class")
    TSubclassOf<AEnemyCharacter> BossClass;

    // 최대 유지 몬스터 수
    // 현재 살아있는 일반 몬스터 수가 이 값보다 적으면 계속 스폰
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Spawn")
    int32 MaxAliveEnemies = 20;

    // 몬스터 스폰 체크 간격
    // 몇 초마다 몬스터 수를 확인해서 부족하면 1마리씩 보충
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Spawn")
    float SpawnInterval = 1.0f;

    // 여러 스폰 위치
    // 맵에 배치한 SpawnPoint Actor들을 배열로 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Spawn")
    TArray<AActor*> SpawnPoints;

    // 스폰 반경
    // 선택된 SpawnPoint 기준으로 주변 랜덤 위치에 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Spawn")
    float SpawnRadius = 100.0f;

    // =========================
    // Kill Count Setting
    // =========================

    // 일반 몬스터 강화 간격
    // 예: 50이면 50킬마다 다음 스폰 몬스터 스탯 상승
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Kill Count")
    int32 StatUpgradeInterval = 50;

    // 보스 등장 간격
    // 예: 150이면 150킬마다 보스 1마리 스폰
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Kill Count")
    int32 BossSpawnInterval = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Boss")
    bool bIsBossSpawner = false;

    void TrySpawnBossFromBossSpawner();

    // =========================
    // Enemy Data Setting
    // =========================

    // 스폰된 Enemy에게 전달할 DataTable
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Enemy Data")
    UDataTable* EnemyDataTable;

    // 자동선택 기능 추가로 나중에 삭제 가능
    // DataTable에서 사용할 일반 몬스터 Row 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Enemy Data")
    FName EnemyDataRowName;

    // DataTable에서 사용할 보스 몬스터 Row 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Enemy Data")
    FName BossDataRowName;

    // =========================
    // Runtime Data
    // =========================

    // 현재 살아있는 일반 몬스터 수
    // 일반 몬스터 스폰 시 증가 / 사망 시 감소
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner|Runtime")
    int32 CurrentAliveEnemies = 0;

    // 전체 일반 몬스터 처치 수
    static int32 GlobalKillCount;

    // 전체 보스 처치 수
    static int32 GlobalBossKillCount;

    // 전체 몬스터 강화 단계
    static int32 GlobalStatLevel;

    static bool bGlobalCounterInitialized;

    // 몬스터 스폰 반복 체크용 타이머
    FTimerHandle SpawnTimerHandle;

    // =========================
    // Upgrade Setting
    // =========================

    // 업그레이드가 제공되는 첫 목표 킬 수들
    // 예: 20, 40, 60, 100킬 달성 시 업그레이드 선택 제공
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    TArray<int32> UpgradeKillGoals = { 20, 40, 60, 100 };

    // UpgradeKillGoals 이후 반복 업그레이드 간격
    // 기본값 100이면 200, 300, 400... 킬마다 업그레이드 선택 제공
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    int32 RepeatUpgradeKillInterval = 100;

    // =========================
    // Spawn Function
    // =========================

    // 몬스터 수 유지 체크 함수
    // SpawnInterval마다 호출
    void MaintainEnemyCount();

    // 일반 몬스터 1마리 생성
    void SpawnEnemy();

    // 보스 몬스터 1마리 생성
    void SpawnBoss();

    // 현재 강화 단계에 맞는 EnemyData RowName 반환
    FName GetEnemyRowNameByStatLevel(bool bIsJumpEnemy) const;

    // 현재 살아있는 몬스터들에게 현재 강화 단계 데이터 다시 적용
    void ApplyCurrentLevelToAliveEnemies();

    // 목표 킬 수 달성 시 업그레이드 선택을 띄울지 확인
    void CheckUpgradeReward();

public:
    // =========================
    // Kill Count Function
    // =========================

    // 일반 몬스터 사망 시 EnemyCharacter에서 호출
    void OnEnemyKilled();

    // 보스 몬스터 사망 시 BossCharacter에서 호출 예정
    void OnBossKilled();

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // =========================
    // UI Getter Function
    // =========================

    // UI에서 현재 일반 몬스터 처치 수 확인
    UFUNCTION(BlueprintCallable, Category = "Spawner|UI")
    int32 GetKillCount() const;

    // UI에서 현재 보스 처치 수 확인
    UFUNCTION(BlueprintCallable, Category = "Spawner|UI")
    int32 GetBossKillCount() const;

    // UI에서 현재 강화 단계 확인
    UFUNCTION(BlueprintCallable, Category = "Spawner|UI")
    int32 GetCurrentStatLevel() const;
};
