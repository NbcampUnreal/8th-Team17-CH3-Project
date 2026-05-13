#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillBase.generated.h"

UCLASS(Abstract, Blueprintable)
class TEMAPROJECT03_API USkillBase : public UObject
{
    GENERATED_BODY()

public:
    // 스킬 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FString SkillName = TEXT("Skill");

    // 스킬 재사용 대기 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float Cooldown = 10.0f;

    // 스킬 데미지 저장용
    float SkillDamage = 0.0f;

    // 현재 스킬 쿨타임 여부
    bool bIsOnCooldown = false;

    // 쿨타임 시간 관리 위한 핸들
    FTimerHandle CooldownTimerHandle;

    // 현재 스킬 사용 할 수 있나 체크
    bool CanUseSkill() const { return !bIsOnCooldown; }

    // 스킬 동작 정의하는 가상 함수
    // true 반환 시에만 쿨타임 시작
    UFUNCTION(BlueprintCallable, Category = "Skill")
    virtual bool ActivateSkill(AActor* Owner) { return false; }

    // 쿨타임 시작
    void StartCooldown(UWorld* World);

private:
    // 쿨타임 끝나면 상태 초기화
    void ResetCooldown();
};
