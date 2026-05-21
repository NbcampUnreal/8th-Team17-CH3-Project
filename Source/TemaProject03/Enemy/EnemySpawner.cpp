// EnemySpawner.cpp

#include "EnemySpawner.h"
#include "EnemyCharacter.h"
#include "TemaProject03/Gimmick/Portal.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "TemaProject03/Gimmick/RoomManager.h"

AEnemySpawner::AEnemySpawner()
{
    // Tick 사용 안 함
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // =========================
    // Portal Init
    // =========================

    // 게임 시작 시 포탈 비활성화
    AActor* PortalActor = UGameplayStatics::GetActorOfClass(GetWorld(), APortal::StaticClass());
    if (APortal* Portal = Cast<APortal>(PortalActor))
    {
        Portal->SetPortalActive(false);
    }

    // =========================
    // Runtime Data Init
    // =========================

    CurrentAliveEnemies = 0;
    KillCount = 0;

    // =========================
    // Spawn Timer Start
    // =========================

    // 일정 시간마다 현재 몬스터 수를 확인해서 부족하면 스폰
    GetWorldTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &AEnemySpawner::MaintainEnemyCount,
        SpawnInterval,
        true
    );

    // 시작하자마자 한 번 채우기
    MaintainEnemyCount();
}

void AEnemySpawner::MaintainEnemyCount()
{
    // 목표 처치 수를 달성했으면 더 이상 스폰하지 않음
    if (KillCount >= TargetKillCount)
    {
        return;
    }

    // 현재 살아있는 몬스터가 최대 유지 수보다 적으면 1마리씩 보충
    if (CurrentAliveEnemies < MaxAliveEnemies)
    {
        SpawnEnemy();
    }
}

void AEnemySpawner::SpawnEnemy()
{
    // =========================
    // Validation Check
    // =========================

    if (!EnemyClass)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyClass is NULL"));
        return;
    }

    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnPoints is Empty"));
        return;
    }

    // =========================
    // Random Spawn Point
    // =========================

    const int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
    AActor* SelectedPoint = SpawnPoints[RandomIndex];

    if (!SelectedPoint)
    {
        return;
    }

    // =========================
    // Spawn Parameter
    // =========================

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // =========================
    // Spawn Location
    // =========================

    FVector RandomOffset = FMath::VRand() * SpawnRadius;
    RandomOffset.Z = 0.0f;

    const FVector SpawnLocation =
        SelectedPoint->GetActorLocation() + RandomOffset;

    const FRotator SpawnRotation =
        SelectedPoint->GetActorRotation();

    // =========================
    // Spawn Enemy
    // =========================

    AEnemyCharacter* SpawnedEnemy =
        GetWorld()->SpawnActor<AEnemyCharacter>(
            EnemyClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

    if (!SpawnedEnemy)
    {
        return;
    }

    // =========================
    // Enemy Data Apply
    // =========================

    SpawnedEnemy->EnemyDataTable = EnemyDataTable;
    SpawnedEnemy->EnemyDataRowName = EnemyDataRowName;
    SpawnedEnemy->ApplyEnemyData();

    // AI Controller 생성
    SpawnedEnemy->SpawnDefaultController();

    // 현재 살아있는 몬스터 수 증가
    CurrentAliveEnemies++;

    UE_LOG(LogTemp, Warning,
        TEXT("Enemy Spawned / Alive: %d / Kill: %d / Target: %d"),
        CurrentAliveEnemies,
        KillCount,
        TargetKillCount
    );
}

void AEnemySpawner::OnEnemyKilled()
{
    // =========================
    // Count Update
    // =========================

    CurrentAliveEnemies = FMath::Max(0, CurrentAliveEnemies - 1);
    KillCount++;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            999,    // 같은 ID라서 한 줄 갱신
            2.0f,
            FColor::Yellow,
            FString::Printf(
                TEXT("Kill Count : %d / %d   Alive : %d"),
                KillCount,
                TargetKillCount,
                CurrentAliveEnemies
            )
        );
    }

    // =========================
    // Upgrade Milestone
    // =========================

    if (KillCount == 30 || KillCount == 50 || KillCount == 70)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Upgrade Choice Trigger / KillCount: %d"),
            KillCount
        );

        // TODO:
        // 여기서 업그레이드 UI 담당 쪽 함수나 이벤트 호출
        // 예: GameMode, PlayerController, UpgradeManager 등
    }

    // =========================
    // Target Kill Count Check
    // =========================

    if (KillCount >= TargetKillCount)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Target Kill Count Reached! Opening Portal...")
        );

        // 더 이상 몬스터 스폰하지 않도록 타이머 정지
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

        // 남아있는 몬스터 제거
        ClearAliveEnemies();

        // 포탈 오픈
        AActor* RoomManagerActor =
            UGameplayStatics::GetActorOfClass(GetWorld(), ARoomManager::StaticClass());

        if (ARoomManager* RoomManager = Cast<ARoomManager>(RoomManagerActor))
        {
            RoomManager->OpenLinkedPortals();
        }
    }
}

void AEnemySpawner::ClearAliveEnemies()
{
    // =========================
    // Find All Enemy
    // =========================

    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AEnemyCharacter::StaticClass(),
        FoundEnemies
    );

    // =========================
    // Destroy All Enemy
    // =========================

    for (AActor* EnemyActor : FoundEnemies)
    {
        if (EnemyActor)
        {
            EnemyActor->Destroy();
        }
    }

    CurrentAliveEnemies = 0;

    UE_LOG(LogTemp, Warning, TEXT("All remaining enemies cleared."));
}
