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
    UPROPERTY(EditAnywhere, Category = "Room Settings")
    TArray<APortal*> LinkedPortals;

    UPROPERTY(EditAnywhere, Category = "Room Settings")
    int32 MonsterCount = 0;

public:
    UFUNCTION(BlueprintCallable)
    void OnMonsterKilled();
};
