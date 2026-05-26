// EnemyAnimInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyTypes.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TEMAPROJECT03_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    // 현재 Enemy 상태
    UPROPERTY(BlueprintReadOnly, Category = "Enemy|State")
    EEnemyState EnemyState = EEnemyState::Idle;

    // 점프 공격 애니메이션 전환용
    UPROPERTY(BlueprintReadOnly, Category = "Enemy|State")
    bool bIsJumpAttack = false;
	
};
