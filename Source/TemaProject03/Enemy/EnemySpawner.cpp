// EnemySpawner.cpp

#include "EnemySpawner.h"
#include "EnemyCharacter.h"
#include "Engine/World.h"

AEnemySpawner::AEnemySpawner()
{
    // Tick 사용 안함
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 스폰카운트와 몬스터의 총 카운트 수량이 같다
    //AActor* PortalActor = UGameplayStatics::GetActorOfClass(GetWorld(), APortal::StaticClass());
    //if (APortal* Portal = Cast<APortal>(PortalActor))
    //{
    //    Portal->SetPortalActive(false);
    //}

    //CurrentEnemyCount = MaxSpawnCount; // for (int32 i = 0; i < MaxSpawnCount; i++) 위에 추가

    // 게임 시작 시 Enemy 생성
    SpawnEnemies();
}

void AEnemySpawner::SpawnEnemies()
{
    // =========================
    // Validation Check
    // =========================

    // 생성할 Enemy 클래스가 없으면 종료
    if (!EnemyClass)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyClass is NULL"));

        return;
    }

    // 등록된 SpawnPoint가 없으면 종료
    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnPoints is Empty"));

        return;
    }

    // =========================
    // Spawn Enemy
    // =========================

    // 최대 스폰 개수만큼 반복 생성
    for (int32 i = 0; i < MaxSpawnCount; i++)
    {
        // =========================
        // Random Spawn Point
        // =========================

        // SpawnPoints 배열 중 랜덤 인덱스 선택
        int32 RandomIndex =
            FMath::RandRange(0, SpawnPoints.Num() - 1);

        // 선택된 SpawnPoint 저장
        AActor* SelectedPoint =
            SpawnPoints[RandomIndex];

        // SpawnPoint가 없으면 다음 반복으로 이동
        if (!SelectedPoint)
        {
            continue;
        }

        // =========================
        // Spawn Parameter
        // =========================

        // Enemy 생성 시 충돌 처리 설정
        // 겹쳐도 가능한 위치에 강제로 생성
        FActorSpawnParameters SpawnParams;

        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // =========================
        // Spawn Enemy
        // =========================

        // SpawnPoint 기준 랜덤 위치 생성
        FVector RandomOffset =
            FMath::VRand() * SpawnRadius;

        // 높이값은 유지
        RandomOffset.Z = 0.0f;

        // 최종 스폰 위치 계산
        FVector SpawnLocation =
            SelectedPoint->GetActorLocation() + RandomOffset;

        // 선택된 SpawnPoint 위치에 Enemy 생성
        AEnemyCharacter* SpawnedEnemy =
            GetWorld()->SpawnActor<AEnemyCharacter>(
                EnemyClass,
                SpawnLocation,
                SelectedPoint->GetActorRotation(),
                SpawnParams
            );

        // Enemy 생성 성공 시
        if (SpawnedEnemy)
        {
            // Enemy AI Controller 생성
            SpawnedEnemy->SpawnDefaultController();

            UE_LOG(LogTemp, Warning, TEXT("Enemy Spawned"));
        }
    }
}

// 생성되는 몬스터가 다 죽으면 포탈 생성
//void AEnemySpawner::OnEnemyKilled()
//{
//    // 적이 죽을 때마다 카운트 감소
//    CurrentEnemyCount--;
//
//    UE_LOG(LogTemp, Log, TEXT("Enemy Killed. Remaining: %d"), CurrentEnemyCount);
//
//
//    // 모든 적을 처치했는지 확인
//    if (CurrentEnemyCount <= 0)
//    {
//        UE_LOG(LogTemp, Error, TEXT("All Enemies Defeated! Opening Portal..."));
//
//        // 월드에서 포탈을 찾아 OpenPortal 함수 실행
//        AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), APortal::StaticClass());
//        APortal* TargetPortal = Cast<APortal>(FoundActor);
//
//        if (TargetPortal)
//        {
//            TargetPortal->OpenPortal();
//        }
//    }
//}
