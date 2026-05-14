#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillBase.h"
#include "SkillComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEMAPROJECT03_API USkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USkillComponent();

protected:
    virtual void BeginPlay() override;

public:
    // 사용할 스킬 클래스 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    TSubclassOf<USkillBase> SkillClass;

    // 생성된 스킬 객체 포인터
    UPROPERTY()
    USkillBase* CurrentSkill;

    bool UseSkill();

    // 남은 쿨타임 시간 반환 (UI 연결용)
    float GetRemainingCooldown() const;
};
