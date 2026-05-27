// BossEnemyCharacter.cpp

#include "BossEnemyCharacter.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemySpawner.h"
#include "Kismet/GameplayStatics.h"

void ABossEnemyCharacter::UpdateEnemyState()
{
    Super::UpdateEnemyState();

    if (EnemyState == EEnemyState::Dead)
    {
        BossState = EBossState::Dead;
        return;
    }

    if (EnemyState == EEnemyState::Hit)
    {
        BossState = EBossState::Hit;
        return;
    }

    if (bIsIntroPlaying)
    {
        BossState = EBossState::Intro;
        return;
    }

    if (bIsUsingPattern)
    {
        return;
    }

    if (EnemyState == EEnemyState::Idle)
    {
        BossState = EBossState::Idle;
    }
    else if (EnemyState == EEnemyState::Chase)
    {
        BossState = EBossState::Chase;
    }
    else if (EnemyState == EEnemyState::Attack)
    {
        BossState = EBossState::Attack;
    }
}

void ABossEnemyCharacter::HandleEnemyState(float DeltaTime)
{
    if (bIsIntroPlaying)
    {
        return;
    }

    if (bIsUsingPattern)
    {
        if (BossState == EBossState::Dash)
        {
            AddActorWorldOffset(DashDirection * DashSpeed * DeltaTime, true);
            CheckDashHit();
        }

        return;
    }

    AActor* Target = TargetPlayer;

    if (!Target)
    {
        Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }

    if (Target && bCanDash && bCanUsePattern)
    {
        const float DistanceToPlayer = FVector::Dist(
            GetActorLocation(),
            Target->GetActorLocation()
        );

        if (DistanceToPlayer >= DashMinRange &&
            DistanceToPlayer <= DashMaxRange)
        {
            StartDash();
            return;
        }
    }

    Super::HandleEnemyState(DeltaTime);
}

void ABossEnemyCharacter::TryAttack()
{
    if (bIsIntroPlaying || bIsUsingPattern)
    {
        return;
    }

    Super::TryAttack();
}

void ABossEnemyCharacter::OnHitEnd()
{
    Super::OnHitEnd();

    if (EnemyState != EEnemyState::Dead)
    {
        BossState = EBossState::Chase;
    }

    UE_LOG(LogTemp, Warning, TEXT("Boss Hit End"));
}

void ABossEnemyCharacter::ApplyDamage(float DamageAmount)
{
    if (bBossKillCounted)
    {
        return;
    }

    Super::ApplyDamage(DamageAmount);

    if (EnemyState == EEnemyState::Dead)
    {
        BossState = EBossState::Dead;
        bIsUsingPattern = false;
        bBossKillCounted = true;

        if (OwnerSpawner)
        {
            OwnerSpawner->OnBossKilled();
        }

        return;
    }

    if (EnemyState == EEnemyState::Hit)
    {
        BossState = EBossState::Hit;
        bIsUsingPattern = false;
        return;
    }
}

void ABossEnemyCharacter::ResetPatternDelay()
{
    bCanUsePattern = true;
}

void ABossEnemyCharacter::DestroyEnemy()
{
    Destroy();
}

void ABossEnemyCharacter::OnAttackEnd()
{
    if (bIsUsingPattern)
    {
        return;
    }

    bIsAttacking = false;
    bCanAttack = false;
    bCanUsePattern = false;

    GetWorldTimerManager().SetTimer(
        AttackCooldownTimerHandle,
        this,
        &ABossEnemyCharacter::ResetAttack,
        AttackCooldown,
        false
    );

    GetWorldTimerManager().SetTimer(
        PatternDelayTimerHandle,
        this,
        &ABossEnemyCharacter::ResetPatternDelay,
        PatternDelay,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("Boss Attack End"));
}

void ABossEnemyCharacter::OnBossIntroEnd()
{
    bIsIntroPlaying = false;
    BossState = EBossState::Idle;

    UE_LOG(LogTemp, Warning, TEXT("Boss Intro End"));
}

// =========================
// Dash Pattern
// =========================

void ABossEnemyCharacter::StartDash()
{
    if (!bCanDash || bIsUsingPattern)
    {
        return;
    }

    AActor* Target = TargetPlayer;

    if (!Target)
    {
        Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }

    if (!Target)
    {
        return;
    }

    bCanDash = false;
    bCanUsePattern = false;
    bIsUsingPattern = true;
    bDashHitPlayer = false;
    BossState = EBossState::Dash;

    FVector Direction = Target->GetActorLocation() - GetActorLocation();
    Direction.Z = 0.f;
    DashDirection = Direction.GetSafeNormal();

    if (DashDirection.IsNearlyZero())
    {
        DashDirection = GetActorForwardVector();
    }

    SetActorRotation(DashDirection.Rotation());

    GetWorldTimerManager().SetTimer(
        DashTimerHandle,
        this,
        &ABossEnemyCharacter::EndDash,
        DashDuration,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("Boss Dash Start"));
}

void ABossEnemyCharacter::CheckDashHit()
{
    if (bDashHitPlayer)
    {
        return;
    }

    AActor* Target = TargetPlayer;

    if (!Target)
    {
        Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }

    APlayerCharacter* Player = Cast<APlayerCharacter>(Target);
    if (!Player)
    {
        return;
    }

    const float DistanceToPlayer = FVector::Dist(
        GetActorLocation(),
        Player->GetActorLocation()
    );

    if (DistanceToPlayer > AttackRange + 100.0f)
    {
        return;
    }

    bDashHitPlayer = true;

    Player->ApplyDamage(DashDamage);

    FVector KnockbackDirection = Player->GetActorLocation() - GetActorLocation();
    KnockbackDirection.Z = 0.f;
    KnockbackDirection = KnockbackDirection.GetSafeNormal();

    if (KnockbackDirection.IsNearlyZero())
    {
        KnockbackDirection = GetActorForwardVector();
    }

    const FVector LaunchVelocity =
        KnockbackDirection * DashKnockbackPower +
        FVector::UpVector * DashKnockbackUpPower;

    Player->LaunchCharacter(LaunchVelocity, true, true);

    UE_LOG(LogTemp, Warning, TEXT("Boss Dash Hit Player"));
}

void ABossEnemyCharacter::EndDash()
{
    bIsUsingPattern = false;
    BossState = EBossState::Chase;

    bCanAttack = false;

    GetWorldTimerManager().SetTimer(
        AttackCooldownTimerHandle,
        this,
        &ABossEnemyCharacter::ResetAttack,
        AttackCooldown,
        false
    );

    GetWorldTimerManager().SetTimer(
        DashCooldownTimerHandle,
        this,
        &ABossEnemyCharacter::ResetDash,
        DashCooldown,
        false
    );

    GetWorldTimerManager().SetTimer(
        PatternDelayTimerHandle,
        this,
        &ABossEnemyCharacter::ResetPatternDelay,
        PatternDelay,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("Boss Dash End"));
}

void ABossEnemyCharacter::ResetDash()
{
    bCanDash = true;

    UE_LOG(LogTemp, Warning, TEXT("Boss Dash Ready"));
}
