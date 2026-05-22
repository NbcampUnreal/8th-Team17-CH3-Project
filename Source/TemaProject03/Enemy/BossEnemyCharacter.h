// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "BossEnemyCharacter.generated.h"

UENUM(BlueprintType)
enum class EBossState : uint8
{
    Idle,
    Intro,
    Chase,
    Attack,
    Dash,
    JumpSmash,
    PhaseChange,
    Hit,
    Dead
};

UCLASS()
class TEMAPROJECT03_API ABossEnemyCharacter : public AEnemyCharacter
{
	GENERATED_BODY()


    protected:
        virtual void UpdateEnemyState() override;
        virtual void HandleEnemyState(float DeltaTime) override;
        virtual void TryAttack() override;
        virtual void OnHitEnd() override;
        virtual void ApplyDamage(float DamageAmount) override;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
        EBossState BossState = EBossState::Idle;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Combat")
        bool bIsUsingPattern = false;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
        bool bBossIntroPlayed = true;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
        bool bIsIntroPlaying = true;

    public:
        virtual void OnAttackEnd() override;

        UFUNCTION(BlueprintCallable)
        void OnBossIntroEnd();
};
