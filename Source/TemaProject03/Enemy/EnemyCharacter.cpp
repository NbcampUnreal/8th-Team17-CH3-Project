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
#include "TemaProject03/Item/Item.h"

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

    ApplyEnemyData();
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateEnemyState();
    HandleEnemyState(DeltaTime);
}

void AEnemyCharacter::RecoverFromHit()
{
    if (EnemyState != EEnemyState::Hit)
    {
        return;
    }

    SetEnemyState(EEnemyState::Chase);
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
    AttackType = Data->AttackType;

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

    if (DetectSphere)
    {
        DetectSphere->SetSphereRadius(DetectRange);
    }

    if (AttackSphere)
    {
        AttackSphere->SetSphereRadius(AttackRange);
    }

    UE_LOG(LogTemp, Warning,
        TEXT("Sphere Applied / Detect: %.1f / Attack: %.1f"),
        DetectSphere ? DetectSphere->GetUnscaledSphereRadius() : -1.0f,
        AttackSphere ? AttackSphere->GetUnscaledSphereRadius() : -1.0f
    );
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

        //UE_LOG(LogTemp, Warning, TEXT("Player Entered Detect Range: %s"), *OtherActor->GetName());
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

    //UE_LOG(LogTemp, Warning, TEXT("Player Seen By PawnSensing: %s"), *SeenPawn->GetName());
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
        AIController->StopMovement();

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->StopMovementImmediately();
        }

        return;
    }

    // 공격 범위 밖이면 계속 플레이어 추격
    AIController->MoveToActor(TargetPlayer, 5.0f);
}

void AEnemyCharacter::StopEnemyMovement()
{
    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->StopMovement();
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
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
    if (!bCanAttack || bIsAttacking || !TargetPlayer)
    {
        return;
    }

    switch (AttackType)
    {
    case EEnemyAttackType::Melee:

        bIsAttacking = true;
        bCanAttack = false;

        UE_LOG(LogTemp, Warning, TEXT("Melee Attack Start"));

        // 여기서 PerformMeleeAttack 호출하지 않음
        // 데미지는 AnimNotify_MeleeHit에서 PerformMeleeAttack 호출
        break;
    }
}

bool AEnemyCharacter::IsTargetInJumpAttackRange() const
{
    if (!TargetPlayer)
    {
        return false;
    }

    FVector EnemyLocation = GetActorLocation();
    FVector TargetLocation = TargetPlayer->GetActorLocation();

    EnemyLocation.Z = 0.0f;
    TargetLocation.Z = 0.0f;

    const float Distance = FVector::Dist(EnemyLocation, TargetLocation);

    return Distance >= JumpAttackMinRange && Distance <= JumpAttackMaxRange;
}

void AEnemyCharacter::TryJumpAttack()
{
    if (!bUseJumpAttack || !bCanJumpAttack || bIsAttacking || bIsJumpAttacking || !TargetPlayer)
    {
        return;
    }

    bIsAttacking = true;
    bIsJumpAttacking = true;
    bCanJumpAttack = false;

    StopEnemyMovement();

    JumpAttackElapsedTime = 0.0f;
    JumpAttackStartLocation = GetActorLocation();

    FVector EnemyLocation = GetActorLocation();
    FVector TargetLocation = TargetPlayer->GetActorLocation();

    FVector ToEnemy = EnemyLocation - TargetLocation;
    ToEnemy.Z = 0.0f;
    ToEnemy.Normalize();

    JumpAttackTargetLocation = TargetLocation + (ToEnemy * JumpAttackLandingOffset);
    JumpAttackTargetLocation.Z = EnemyLocation.Z;

    if (JumpAttackMontage)
    {
        PlayAnimMontage(JumpAttackMontage);
    }

    UE_LOG(LogTemp, Warning, TEXT("Jump Attack Start"));
}

void AEnemyCharacter::HandleJumpAttack(float DeltaTime)
{
    if (!bIsJumpAttacking)
    {
        return;
    }

    JumpAttackElapsedTime += DeltaTime;

    const float Alpha = FMath::Clamp(JumpAttackElapsedTime / JumpAttackMoveTime, 0.0f, 1.0f);

    FVector NewLocation = FMath::Lerp(
        JumpAttackStartLocation,
        JumpAttackTargetLocation,
        Alpha
    );

    // 포물선 느낌용 Z 보정
    const float JumpHeight = 250.0f;
    NewLocation.Z += FMath::Sin(Alpha * PI) * JumpHeight;

    SetActorLocation(NewLocation, true);

    if (Alpha >= 1.0f)
    {
        PerformJumpAttack();
        OnJumpAttackEnd();
    }
}

void AEnemyCharacter::PerformJumpAttack()
{
    if (!TargetPlayer)
    {
        return;
    }

    const float Distance = FVector::Dist2D(
        GetActorLocation(),
        TargetPlayer->GetActorLocation()
    );

    if (Distance > JumpAttackRadius)
    {
        UE_LOG(LogTemp, Warning, TEXT("Jump Attack Miss"));
        return;
    }

    APlayerCharacter* Player = Cast<APlayerCharacter>(TargetPlayer);

    if (Player)
    {
        Player->ApplyDamage(JumpAttackDamage);

        UE_LOG(LogTemp, Warning,
            TEXT("Jump Attack Hit / Damage: %.1f"),
            JumpAttackDamage
        );
    }
}

void AEnemyCharacter::OnJumpAttackEnd()
{
    bIsJumpAttacking = false;
    bIsAttacking = false;

    StopEnemyMovement();

    GetWorldTimerManager().SetTimer(
        JumpAttackCooldownTimerHandle,
        this,
        &AEnemyCharacter::ResetJumpAttack,
        JumpAttackCooldown,
        false
    );

    SetEnemyState(EEnemyState::Chase);

    UE_LOG(LogTemp, Warning, TEXT("Jump Attack End"));
}

void AEnemyCharacter::ResetJumpAttack()
{
    bCanJumpAttack = true;

    UE_LOG(LogTemp, Warning, TEXT("Jump Attack Cooldown End"));
}


// 근접 공격 
void AEnemyCharacter::PerformMeleeAttack()
{
    if (!TargetPlayer || !IsTargetInAttackRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("Melee Failed: No Target or Out of Range"));
        return;
    }

    APlayerCharacter* Player = Cast<APlayerCharacter>(TargetPlayer);

    if (Player)
    {
        Player->ApplyDamage(AttackPower);
        UE_LOG(LogTemp, Warning, TEXT("Melee Hit Player / Damage: %.1f"), AttackPower);
    }
}

void AEnemyCharacter::OnAttackEnd()
{
    bIsAttacking = false;
    bCanAttack = false;

    StopEnemyMovement();

    GetWorldTimerManager().SetTimer(
        AttackCooldownTimerHandle,
        this,
        &AEnemyCharacter::ResetAttack,
        AttackCooldown,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("Attack End / Wait Cooldown"));
}

void AEnemyCharacter::DestroyEnemy()
{
    if (OwnerSpawner)
    {
        OwnerSpawner->OnEnemyKilled();
    }

    TryDropHealthItem();

    Destroy();
}

void AEnemyCharacter::TryDropHealthItem()
{
    if (!HealthDropItemClass || !GetWorld())
    {
        return;
    }

    if (FMath::FRand() > HealthDropChance)
    {
        return;
    }

    FVector DropLocation = GetActorLocation();
    DropLocation.Z += DropLocationZOffset;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    GetWorld()->SpawnActor<AItem>(
        HealthDropItemClass,
        DropLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );
}

void AEnemyCharacter::OnHitEnd()
{
    if (EnemyState != EEnemyState::Hit)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(HitRecoverTimerHandle);

    SetEnemyState(EEnemyState::Chase);
}

void AEnemyCharacter::ResetAttack()
{
    bCanAttack = true;

    UE_LOG(LogTemp, Warning, TEXT("Attack Cooldown End"));
}

void AEnemyCharacter::UpdateEnemyState()
{
    if (EnemyState == EEnemyState::Dead)
    {
        return;
    }

    if (EnemyState == EEnemyState::Hit)
    {
        return;
    }

    if (bIsJumpAttacking)
    {
        return;
    }

    if (!TargetPlayer)
    {
        SetEnemyState(EEnemyState::Idle);
        return;
    }

    if (bUseJumpAttack && bCanJumpAttack && !bIsAttacking && IsTargetInJumpAttackRange())
    {
        SetEnemyState(EEnemyState::JumpAttack);
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

    case EEnemyState::JumpAttack:
        LookAtTarget(DeltaTime);

        if (bIsJumpAttacking)
        {
            HandleJumpAttack(DeltaTime);
        }
        else
        {
            TryJumpAttack();
        }

        break;

    case EEnemyState::Attack:
        LookAtTarget(DeltaTime);
        StopEnemyMovement();

        if (IsTargetInAttackRange())
        {
            if (bCanAttack && !bIsAttacking)
            {
                TryAttack();
            }
        }
        else
        {
            SetEnemyState(EEnemyState::Chase);
        }

        break;

    case EEnemyState::Hit:
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

    //UE_LOG(LogTemp, Warning, TEXT("Enemy State Changed"));
}

void AEnemyCharacter::ApplyDamage(float DamageAmount)
{
    if (EnemyState == EEnemyState::Dead || CurrentHealth <= 0.0f)
    {
        return;
    }

    CurrentHealth -= DamageAmount;

    UE_LOG(LogTemp, Warning, TEXT("HP: %f"), CurrentHealth);

    if (CurrentHealth <= 0.0f)
    {
        CurrentHealth = 0.0f;

        UE_LOG(LogTemp, Warning, TEXT("Enemy Dead"));

        SetEnemyState(EEnemyState::Dead);

        bIsAttacking = false;
        bCanAttack = false;

        StopEnemyMovement();

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }

        return;
    }

    AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    if (PlayerActor)
    {
        TargetPlayer = PlayerActor;
        bPlayerInDetectRange = true;
        bCanSeePlayer = true;
    }

    if (EnemyState == EEnemyState::Hit)
    {
        return;
    }

    bIsAttacking = false;

    StopEnemyMovement();

    SetEnemyState(EEnemyState::Hit);

    GetWorldTimerManager().ClearTimer(HitRecoverTimerHandle);
    GetWorldTimerManager().SetTimer(
        HitRecoverTimerHandle,
        this,
        &AEnemyCharacter::RecoverFromHit,
        HitRecoverTime,
        false
    );
}

void AEnemyCharacter::SetOwnerSpawner(AEnemySpawner* InSpawner)
{
    OwnerSpawner = InSpawner;
}

EEnemyState AEnemyCharacter::GetCurrentState() const
{
    return EnemyState;
}

void AEnemyCharacter::ApplyEnemyData()
{
    LoadEnemyData();
    InitEnemyStat();
    InitPawnSensing();
}
