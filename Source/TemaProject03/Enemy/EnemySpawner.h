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

    // 스폰할 Enemy 클래스 지정
    // BP_Enemy_A를 Details에서 연결해서 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TSubclassOf<AEnemyCharacter> EnemyClass;

    // 최대 유지 몬스터 수
    // 현재 살아있는 몬스터 수가 이 값보다 적으면 계속 스폰
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    int32 MaxAliveEnemies = 30;

    // 목표 처치 수
    // 이 수치를 달성하면 스폰 종료 + 포탈 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    int32 TargetKillCount = 100;

    // 몬스터 스폰 체크 간격
    // 몇 초마다 현재 몬스터 수를 확인해서 부족하면 스폰
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    float SpawnInterval = 1.0f;

    // 여러 스폰 위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TArray<AActor*> SpawnPoints;

    // 스폰 반경
    // Spawner 기준 주변 랜덤 위치에 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    float SpawnRadius = 100.0f;

    // =========================
    // Enemy Data Setting
    // =========================

    // 스폰된 Enemy에게 전달할 DataTable
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Enemy Data")
    UDataTable* EnemyDataTable;

    // DataTable에서 사용할 Row 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Enemy Data")
    FName EnemyDataRowName;

    // =========================
    // Runtime Data
    // =========================

    // 현재 살아있는 몬스터 수
    // 스폰 시 증가 / 사망 시 감소
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
    int32 CurrentAliveEnemies = 0;

    // 누적 처치 수
    // 몬스터 사망 시 증가
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
    int32 KillCount = 0;

    // 몬스터 스폰 반복 체크용 타이머
    FTimerHandle SpawnTimerHandle;

    // =========================
    // Spawn Function
    // =========================

    // 몬스터 수 유지 체크 함수
    // SpawnInterval마다 호출
    void MaintainEnemyCount();

    // 몬스터 1마리 생성 함수
    void SpawnEnemy();

    // 남아있는 몬스터 전부 제거
    // 목표 KillCount 달성 시 호출
    void ClearAliveEnemies();

public:
    // 몬스터 사망 시 호출
    // EnemyCharacter -> Spawner 로 알림
    void OnEnemyKilled();
};
