#include "SkillBase.h"

// 스킬 쿨타임을 시작시키는 함수
void USkillBase::StartCooldown(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillBase] World is NULL!"));
        return;
    }

    // 쿨타임 상태를 true로 변경하여 중복 실행을 방지
    bIsOnCooldown = true;

    // 언리얼 타이머 매니저를 통해 특정 시간 후에 ResetCooldown 함수가 호출되도록 예약
    // World->GetTimerManager().SetTimer(핸들, 오브젝트, 실행할 함수 주소, 대기 시간, 반복 여부)
    World->GetTimerManager().SetTimer(
        CooldownTimerHandle,
        this,
        &USkillBase::ResetCooldown,
        Cooldown,
        false // 반복하지 않고 한 번만 실행
    );

    UE_LOG(LogTemp, Warning, TEXT("[%s] Cooldown Started: %.1f sec"),
        *SkillName, Cooldown);
}

// 타이머가 종료되었을 때 호출되어 스킬을 다시 사용할 수 있는 상태로 만듦
void USkillBase::ResetCooldown()
{
    bIsOnCooldown = false;
    UE_LOG(LogTemp, Warning, TEXT("[%s] Skill Ready!"), *SkillName);
}
