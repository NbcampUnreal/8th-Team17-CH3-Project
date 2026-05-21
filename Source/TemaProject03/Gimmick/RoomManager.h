#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.h"
#include "RoomManager.generated.h"

class APortal;

UCLASS()
class TEMAPROJECT03_API ARoomManager : public AActor
{
    GENERATED_BODY()

public:
    ARoomManager();

protected:
    // =========================
    // Portal Setting
    // =========================

    // 현재 Room과 연결된 포탈들
    // 클리어 조건 달성 시 활성화
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Settings")
    TArray<APortal*> LinkedPortals;

public:
    // =========================
    // Room Clear Function
    // =========================

    // 방 클리어 시 연결된 포탈 활성화
    UFUNCTION(BlueprintCallable)
    void OpenLinkedPortals();
};
