// EnemyCharacter.cpp

#include "EnemyCharacter.h"
#include "EnemySpawner.h"
#include "EnemyData.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // =========================
    // Detect Sphere
    // =========================

    // 근처 감지용 Sphere 생성
    DetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectSphere"));
    DetectSphere->SetupAttachment(RootComponent);
    DetectSphere->SetSphereRadius(DetectRange);

    // DetectSphere는 물리 충돌 없이 Overlap만 확인
    DetectSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectSphere->SetCollisionObjectType(ECC_WorldDynamic);
    DetectSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DetectSphere->SetGenerateOverlapEvents(true);

    // DetectSphere 안에 Actor가 들어오거나 나갈 때 실행할 함수 연결
    DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnDetectBeginOverlap);
    DetectSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnDetectEndOverlap);

    // =========================
    // Attack Sphere
    // =========================

    // 공격 가능 범위 Sphere 생성
    AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
    AttackSphere->SetupAttachment(RootComponent);
    AttackSphere->SetSphereRadius(AttackRange);

    // =========================
    // Pawn Sensing
    // =========================

    // AI 시야 감지용 컴포넌트 생성
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));

    // 플레이어를 봤을 때 실행할 함수 연결
    PawnSensing->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSeePawn);
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    StartLocation = GetActorLocation();

    LoadEnemyData();

    // 체력, 이동속도 초기화
    InitEnemyStat();

    // 시야 감지 값 초기화
    InitPawnSensing();

    StartLocation = GetActorLocation();
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateEnemyState();
    HandleEnemyState(DeltaTime);
}

void AEnemyCharacter::PatrolMove()
{
    if (!bCanPatrol)
    {
        return;
    }

    AAIController* AIController = Cast<AAIController>(GetController());

    if (!AIController)
    {
        return;
    }

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

    if (!NavSystem)
    {
        return;
    }

    FNavLocation RandomLocation;

    bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(
        StartLocation,
        PatrolRadius,
        RandomLocation
    );

    if (bFoundLocation)
    {
        AIController->MoveToLocation(RandomLocation.Location);

        bCanPatrol = false;

        GetWorldTimerManager().SetTimer(
            PatrolTimerHandle,
            this,
            &AEnemyCharacter::ResetPatrol,
            PatrolWaitTime,
            false
        );
    }
}

void AEnemyCharacter::ResetPatrol()
{
    bCanPatrol = true;
}

void AEnemyCharacter::LoadEnemyData()
{
    if (!EnemyDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyDataTable is not set."));
        return;
    }

    if (EnemyDataRowName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyDataRowName is not set."));
        return;
    }

    FEnemyData* Data = EnemyDataTable->FindRow<FEnemyData>(
        EnemyDataRowName,
        TEXT("Enemy Data Load")
    );

    if (!Data)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyData row not found: %s"), *EnemyDataRowName.ToString());
        return;
    }

    MaxHealth = Data->MaxHealth;
    AttackPower = Data->AttackPower;
    Defense = Data->Defense;
    MoveSpeed = Data->MoveSpeed;

    DetectRange = Data->DetectRange;
    AttackRange = Data->AttackRange;
    AttackCooldown = Data->AttackCooldown;

    PatrolRadius = Data->PatrolRadius;
    PatrolWaitTime = Data->PatrolWaitTime;

    SightRadius = Data->SightRadius;
    VisionAngle = Data->VisionAngle;
    SensingInterval = Data->SensingInterval;

    UE_LOG(LogTemp, Warning,
        TEXT("Loaded Enemy Data: %s / HP: %.1f / Speed: %.1f / Detect: %.1f"),
        *EnemyDataRowName.ToString(),
        MaxHealth,
        MoveSpeed,
        DetectRange
    );
}

void AEnemyCharacter::InitEnemyStat()
{
    // 현재 체력을 최대 체력으로 초기화
    CurrentHealth = MaxHealth;

    // 이동속도 적용
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

    DetectSphere->SetSphereRadius(DetectRange);
    AttackSphere->SetSphereRadius(AttackRange);
}

void AEnemyCharacter::InitPawnSensing()
{
    if (!PawnSensing)
    {
        return;
    }

    // 시야 감지 거리 설정
    PawnSensing->SightRadius = SightRadius;

    // 시야각 설정
    PawnSensing->SetPeripheralVisionAngle(VisionAngle);

    // 감지 체크 주기 설정
    PawnSensing->SensingInterval = SensingInterval;
}

void AEnemyCharacter::OnDetectBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    // OtherActor가 없거나 자기 자신이면 무시
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Player 태그가 있는 Actor만 플레이어로 판단
    if (OtherActor->ActorHasTag(TEXT("Player")))
    {
        // 근처 감지 범위 안에 플레이어가 들어온 상태
        bPlayerInDetectRange = true;

        // 일단 근처 플레이어로 저장
        TargetPlayer = OtherActor;

        UE_LOG(LogTemp, Warning, TEXT("Player Entered Detect Range: %s"), *OtherActor->GetName());
    }
}

void AEnemyCharacter::OnDetectEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    // OtherActor가 없으면 무시
    if (!OtherActor)
    {
        return;
    }

    // 나간 Actor가 현재 타겟 플레이어라면 감지 해제
    if (OtherActor == TargetPlayer)
    {
        // 플레이어가 범위에서 벗어나면 다시 원위치 코드
        /*bPlayerInDetectRange = false;
        bCanSeePlayer = false;*/

        UE_LOG(LogTemp, Warning, TEXT("Player Left Detect Range: %s"), *OtherActor->GetName());

        // 플레이어가 범위에서 벗어나면 다시 원위치 코드
        //TargetPlayer = nullptr;
    }
}

void AEnemyCharacter::OnSeePawn(APawn* SeenPawn)
{
    // 본 Pawn이 없으면 무시
    if (!SeenPawn)
    {
        return;
    }

    // Player 태그가 없으면 무시
    if (!SeenPawn->ActorHasTag(TEXT("Player")))
    {
        return;
    }

    // DetectSphere 안에 들어온 플레이어일 때만 시야 감지 인정
    if (!bPlayerInDetectRange)
    {
        return;
    }

    // 플레이어를 실제 시야로 본 상태
    bCanSeePlayer = true;

    // 현재 타겟 플레이어 확정
    TargetPlayer = SeenPawn;

    UE_LOG(LogTemp, Warning, TEXT("Player Seen By PawnSensing: %s"), *SeenPawn->GetName());
}

bool AEnemyCharacter::CanDetectPlayer() const
{
    return TargetPlayer && (bPlayerInDetectRange || bCanSeePlayer);
}

void AEnemyCharacter::LookAtTarget(float DeltaTime)
{
    if (!TargetPlayer)
    {
        return;
    }

    FVector EnemyLocation = GetActorLocation();
    FVector TargetLocation = TargetPlayer->GetActorLocation();

    // 위아래로 고개 꺾이지 않게 Z값 맞춤
    TargetLocation.Z = EnemyLocation.Z;

    FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(EnemyLocation, TargetLocation);

    FRotator NewRotation = FMath::RInterpTo(
        GetActorRotation(),
        TargetRotation,
        DeltaTime,
        RotationSpeed
    );

    SetActorRotation(NewRotation);
}

void AEnemyCharacter::ChaseTarget()
{
    AAIController* AIController = Cast<AAIController>(GetController());

    if (!AIController || !TargetPlayer)
    {
        return;
    }

    if (IsTargetInAttackRange())
    {
        // 공격 범위 안이면 이동 멈춤
        AIController->StopMovement();
    }
    else
    {
        // 공격 범위 밖이면 플레이어 추적
        AIController->MoveToActor(TargetPlayer, AttackRange - 50.0f);
    }
}

bool AEnemyCharacter::IsTargetInAttackRange() const
{
    if (!TargetPlayer)
    {
        return false;
    }

    FVector EnemyLocation = GetActorLocation();
    FVector TargetLocation = TargetPlayer->GetActorLocation();

    // 높이 차이는 거리 계산에서 제외
    EnemyLocation.Z = 0.0f;
    TargetLocation.Z = 0.0f;

    const float Distance = FVector::Dist(EnemyLocation, TargetLocation);

    return Distance <= AttackRange;
}

void AEnemyCharacter::TryAttack()
{
    if (!bCanAttack || !TargetPlayer)
    {
        return;
    }

    bCanAttack = false;

    switch (AttackType)
    {
    case EEnemyAttackType::Melee:
        UE_LOG(LogTemp, Warning, TEXT("Melee Attack"));
        break;

    case EEnemyAttackType::Ranged:
        UE_LOG(LogTemp, Warning, TEXT("Ranged Attack"));
        break;

    case EEnemyAttackType::Both:
        UE_LOG(LogTemp, Warning, TEXT("Both Attack"));
        break;
    }

    GetWorldTimerManager().SetTimer(
        AttackCooldownTimerHandle,
        this,
        &AEnemyCharacter::ResetAttack,
        AttackCooldown,
        false
    );
}

void AEnemyCharacter::ResetAttack()
{
    bCanAttack = true;
}

void AEnemyCharacter::UpdateEnemyState()
{
    if (!TargetPlayer)
    {
        SetEnemyState(EEnemyState::Idle);
        return;
    }

    if (IsTargetInAttackRange())
    {
        SetEnemyState(EEnemyState::Attack);
        return;
    }

    if (CanDetectPlayer())
    {
        SetEnemyState(EEnemyState::Chase);
        return;
    }

    SetEnemyState(EEnemyState::Idle);
}

void AEnemyCharacter::HandleEnemyState(float DeltaTime)
{
    switch (EnemyState)
    {
    case EEnemyState::Idle:
        PatrolMove();
        break;

    case EEnemyState::Chase:
        LookAtTarget(DeltaTime);
        ChaseTarget();
        break;

    case EEnemyState::Attack:
        LookAtTarget(DeltaTime);
        ChaseTarget(); // AttackRange 안이면 StopMovement 처리
        TryAttack();   // 지금은 로그 출력용
        break;

    case EEnemyState::Dead:
        break;
    }
}

void AEnemyCharacter::SetEnemyState(EEnemyState NewState)
{
    if (EnemyState == NewState)
    {
        return;
    }

    EnemyState = NewState;

    UE_LOG(LogTemp, Warning, TEXT("Enemy State Changed"));
}

void AEnemyCharacter::ApplyDamage(float DamageAmount)
{
    CurrentHealth -= DamageAmount;


    UE_LOG(LogTemp, Warning, TEXT("HP: %f"), CurrentHealth);

    GEngine->AddOnScreenDebugMessage(
        -1,
        2.f,
        FColor::Red,
        FString::Printf(TEXT("HP: %f"), CurrentHealth)
    );

    if (CurrentHealth <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy Dead"));
        AActor* SpawnerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawner::StaticClass());
        if (AEnemySpawner* Spawner = Cast<AEnemySpawner>(SpawnerActor))
        {
            Spawner->OnEnemyKilled();
        }
        Destroy();
    }
}

void AEnemyCharacter::ApplyEnemyData()
{
    LoadEnemyData();
    InitEnemyStat();
    InitPawnSensing();
}
