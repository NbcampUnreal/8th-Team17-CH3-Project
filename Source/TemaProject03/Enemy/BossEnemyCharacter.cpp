// Fill out your copyright notice in the Description page of Project Settings.


#include "BossEnemyCharacter.h"
#include "TimerManager.h"

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
    Super::HandleEnemyState(DeltaTime);
}

void ABossEnemyCharacter::TryAttack()
{
    Super::TryAttack();
}

void ABossEnemyCharacter::OnHitEnd()
{
    Super::OnHitEnd();

    BossState = EBossState::Chase;

    UE_LOG(LogTemp, Warning, TEXT("Boss Hit End"));
}

void ABossEnemyCharacter::ApplyDamage(float DamageAmount)
{
    Super::ApplyDamage(DamageAmount);

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
}

void ABossEnemyCharacter::OnAttackEnd()
{
    bIsAttacking = false;

    GetWorldTimerManager().SetTimer(
        AttackCooldownTimerHandle,
        this,
        &ABossEnemyCharacter::ResetAttack,
        AttackCooldown,
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
