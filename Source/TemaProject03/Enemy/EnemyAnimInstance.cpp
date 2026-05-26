// EnemyAnimInstance.cpp


#include "EnemyAnimInstance.h"
#include "EnemyCharacter.h"

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(TryGetPawnOwner());

    if (!Enemy)
    {
        EnemyState = EEnemyState::Idle;
        bIsJumpAttack = false;
        return;
    }

    EnemyState = Enemy->GetCurrentState();

    bIsJumpAttack = EnemyState == EEnemyState::JumpAttack;
}
