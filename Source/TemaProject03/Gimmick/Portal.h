#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Portal.generated.h"

UCLASS()
class TEMAPROJECT03_API APortal : public AActor
{
    GENERATED_BODY()

public:
    APortal();

    // 모든 적 잡았을 때 호출
    void OpenPortal();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* MeshComp;

    UPROPERTY(EditAnywhere, Category = "Portal Settings")
    TArray<FName> NormalLevels;

    UPROPERTY(EditAnywhere, Category = "Portal Settings")
    FName BossLevelName = TEXT("BossMap");

    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

public:
    UFUNCTION(BlueprintCallable)
    void SetPortalActive(bool bActive);

private:
    bool bIsPortalOpen = false;
};
