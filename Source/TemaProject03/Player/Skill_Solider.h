#pragma once

#include "CoreMinimal.h"
#include "SkillBase.h"
#include "Skill_Solider.generated.h"

class ARPGProjectile;

UCLASS(Blueprintable, BlueprintType)
class TEMAPROJECT03_API USkill_Solider : public USkillBase
{
    GENERATED_BODY()

public:
    USkill_Solider();

    // 폭발 반지름 350
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|RPG")
    float ExplosionRadius = 350.0f;

    // RPG 최대 사거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|RPG")
    float FireRange = 10000.0f;

    // 폭발 시 공격 횟수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|RPG")
    int32 HitCount = 3;

    // 탄두 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|RPG")
    float RocketSpeed = 3000.0f;

    // 에디터에서 탄환 할당
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|RPG")
    TSubclassOf<ARPGProjectile> RocketClass;

    virtual bool ActivateSkill(AActor* Owner) override;

private:
    // 플레이어 공격력 * 0.67 계산
    float CalculateDamage(AActor* Owner);
};
