// EnemySpawner.cpp

#include "EnemySpawner.h"
#include "EnemyCharacter.h"
#include "TemaProject03/Gimmick/Portal.h"
#include "TemaProject03/Gimmick/RoomManager.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

int32 AEnemySpawner::GlobalKillCount = 0;
int32 AEnemySpawner::GlobalBossKillCount = 0;
int32 AEnemySpawner::GlobalStatLevel = 0;

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 포탈 시작 비활성화
    AActor* PortalActor = UGameplayStatics::GetActorOfClass(GetWorld(), APortal::StaticClass());
    if (APortal* Portal = Cast<APortal>(PortalActor))
    {
        Portal->SetPortalActive(false);
    }

    // 런타임 초기화
    CurrentAliveEnemies = 0;

    // 생존 시간 시작 기록
    GameStartTime = GetWorld()->GetTimeSeconds();

    // 스폰 타이머 시작
    GetWorldTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &AEnemySpawner::MaintainEnemyCount,
        SpawnInterval,
        true
    );

    MaintainEnemyCount();
}

void AEnemySpawner::MaintainEnemyCount()
{
    if (CurrentAliveEnemies < MaxAliveEnemies)
    {
        SpawnEnemy();
    }
}

void AEnemySpawner::SpawnEnemy()
{
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

    AActor* SelectedPoint = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];

    if (!SelectedPoint)
    {
        return;
    }

    FVector RandomOffset = FMath::VRand() * SpawnRadius;
    RandomOffset.Z = 0.0f;

    FVector SpawnLocation = SelectedPoint->GetActorLocation() + RandomOffset;
    FRotator SpawnRotation = SelectedPoint->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyCharacter* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyCharacter>(
        EnemyClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (!SpawnedEnemy)
    {
        return;
    }

    // 이 몬스터를 소환한 스포너 저장
    SpawnedEnemy->SetOwnerSpawner(this);

    // 데이터 적용
    SpawnedEnemy->EnemyDataTable = EnemyDataTable;
    SpawnedEnemy->EnemyDataRowName = GetEnemyRowNameByStatLevel(bSpawnJumpEnemy);
    SpawnedEnemy->ApplyEnemyData();

    SpawnedEnemy->SpawnDefaultController();

    CurrentAliveEnemies++;

    UE_LOG(LogTemp, Warning,
        TEXT("Enemy Spawned / Alive: %d / Kill: %d / StatLevel: %d"),
        CurrentAliveEnemies,
        GlobalKillCount,
        GlobalStatLevel
    );
}

void AEnemySpawner::SpawnBoss()
{
    if (!BossClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BossClass is NULL"));
        return;
    }

    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnPoints is Empty"));
        return;
    }

    AActor* SelectedPoint = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];

    if (!SelectedPoint)
    {
        return;
    }

    FVector SpawnLocation = SelectedPoint->GetActorLocation();
    FRotator SpawnRotation = SelectedPoint->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyCharacter* SpawnedBoss = GetWorld()->SpawnActor<AEnemyCharacter>(
        BossClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (!SpawnedBoss)
    {
        return;
    }

    SpawnedBoss->EnemyDataTable = EnemyDataTable;
    SpawnedBoss->EnemyDataRowName = BossDataRowName;
    SpawnedBoss->ApplyEnemyData();

    SpawnedBoss->SpawnDefaultController();

    UE_LOG(LogTemp, Error,
        TEXT("Boss Spawned! KillCount: %d"),
        GlobalKillCount
    );
}

FName AEnemySpawner::GetEnemyRowNameByStatLevel(bool bIsJumpEnemy) const
{
    const FString Prefix = bIsJumpEnemy ? TEXT("Enemy_B") : TEXT("Enemy_A");

    const int32 Level = FMath::Clamp(GlobalStatLevel + 1, 1, 4);

    return FName(*FString::Printf(TEXT("%s_Lv%d"), *Prefix, Level));
}

void AEnemySpawner::ApplyCurrentLevelToAliveEnemies()
{
    TArray<AActor*> FoundEnemies;

    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AEnemyCharacter::StaticClass(),
        FoundEnemies
    );

    const FName NewRowName = GetEnemyRowNameByStatLevel(bSpawnJumpEnemy);

    for (AActor* EnemyActor : FoundEnemies)
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(EnemyActor);

        if (!Enemy)
        {
            continue;
        }

        Enemy->EnemyDataTable = EnemyDataTable;
        Enemy->EnemyDataRowName = NewRowName;
        Enemy->ApplyEnemyData();
    }

    UE_LOG(LogTemp, Warning,
        TEXT("All Alive Enemies Updated To Row: %s"),
        *NewRowName.ToString()
    );
}

void AEnemySpawner::OnEnemyKilled()
{
    CurrentAliveEnemies = FMath::Max(0, CurrentAliveEnemies - 1);
    GlobalKillCount++;

    // 50킬마다 전체 몬스터 강화
    if (GlobalKillCount % StatUpgradeInterval == 0)
    {
        GlobalStatLevel++;

        // 기존에 몬스터가 존재하면 그 기존 몬스터를 강화한 상태로 변경
        // 현재는 뺀 상태 넣으면 데이터 테이블 몬스터가 꼬일수 있음
        //ApplyCurrentLevelToAliveEnemies();

        UE_LOG(LogTemp, Warning,
            TEXT("Enemy Stat Up! Global Level: %d"),
            GlobalStatLevel
        );
    }

    // 150킬마다 보스 등장
    if (GlobalKillCount % BossSpawnInterval == 0)
    {
        SpawnBoss();
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            999,
            2.0f,
            FColor::Yellow,
            FString::Printf(
                TEXT("Kill: %d | Boss: %d | Alive: %d | StatLv: %d"),
                GlobalKillCount,
                GlobalBossKillCount,
                CurrentAliveEnemies,
                GlobalStatLevel
            )
        );
    }
}

void AEnemySpawner::OnBossKilled()
{
    GlobalBossKillCount++;

    UE_LOG(LogTemp, Warning,
        TEXT("Boss Killed! Total Boss Kill: %d"),
        GlobalBossKillCount
    );
}

int32 AEnemySpawner::GetKillCount() const
{
    return GlobalKillCount;
}

int32 AEnemySpawner::GetBossKillCount() const
{
    return GlobalBossKillCount;
}

float AEnemySpawner::GetSurvivalTime() const
{
    if (!GetWorld())
    {
        return 0.0f;
    }

    return GetWorld()->GetTimeSeconds() - GameStartTime;
}

int32 AEnemySpawner::GetCurrentStatLevel() const
{
    return GlobalStatLevel;
}
