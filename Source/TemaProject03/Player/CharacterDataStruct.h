#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FCharacterStatRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float Health = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float AttackDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float Defense = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MoveSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float CriticalMultiplier = 0.0f;
};
