// EnemySpawner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

    // 최대 스폰 개수
    // BeginPlay에서 반복 생성할 Enemy 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    int32 MaxSpawnCount = 5;

    // 여러 스폰 위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TArray<AActor*> SpawnPoints;

    // 스폰 반경
    // Spawner 기준 주변 랜덤 위치에 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    float SpawnRadius = 500.0f;

    // =========================
    // Spawn Function
    // =========================

    // Enemy 생성 함수
    // BeginPlay에서 호출
    void SpawnEnemies();

public:
    void OnEnemyKilled();

protected:
    int32 CurrentEnemyCount = 0;
};
