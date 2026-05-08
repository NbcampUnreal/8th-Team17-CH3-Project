#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

UCLASS()
class TEMAPROJECT03_API UMyGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // 현재 몇 번째 방인지 (1~5) 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
    int32 CurrentStageCount = 0;

    // 보스 전까지 필요한 총 방 개수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
    int32 MaxStagesBeforeBoss = 5;

    // 아직 방문하지 않은 남은 레벨 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
    TArray<FName> RemainingLevels;
};
