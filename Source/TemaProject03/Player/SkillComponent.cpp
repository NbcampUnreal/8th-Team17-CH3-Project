#include "SkillComponent.h"

USkillComponent::USkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USkillComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!SkillClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillComp] SkillClass is not set!"));
        return;
    }

    CurrentSkill = NewObject<USkillBase>(this, SkillClass);
    if (CurrentSkill)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillComp] Skill Loaded: %s"),
            *CurrentSkill->SkillName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillComp] Failed to create Skill instance!"));
    }
}

bool USkillComponent::UseSkill()
{
    if (!CurrentSkill)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillComp] No Skill Assigned!"));
        return false;
    }

    if (!CurrentSkill->CanUseSkill())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillComp] On Cooldown! Remaining: %.1f sec"),
            GetRemainingCooldown());
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillComp] Owner is NULL!"));
        return false;
    }

    // 스킬 발동이 성공했을 때만 쿨타임 시작
    if (!CurrentSkill->ActivateSkill(Owner))
    {
        UE_LOG(LogTemp, Warning, TEXT("[SkillComp] Skill Activate Failed!"));
        return false;
    }

    CurrentSkill->StartCooldown(GetWorld());
    return true;
}

float USkillComponent::GetRemainingCooldown() const
{
    if (!CurrentSkill || !CurrentSkill->bIsOnCooldown) return 0.0f;
    return GetWorld()->GetTimerManager().GetTimerRemaining(
        CurrentSkill->CooldownTimerHandle);
}
