#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBox.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AWeaponBase;
class APlayerCharacter;

UCLASS()
class TEMAPROJECT03_API AWeaponBox : public AActor
{
    GENERATED_BODY()

public:
    AWeaponBox();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponBox")
    UBoxComponent* BoxCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponBox")
    UStaticMeshComponent* BoxMesh;

    // 박스에서 랜덤으로 뽑을 무기 BP 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponBox")
    TArray<TSubclassOf<AWeaponBase>> WeaponPool;

    // 이번에 박스에서 나온 무기 인스턴스 목록
    UPROPERTY(BlueprintReadOnly, Category = "WeaponBox")
    TArray<AWeaponBase*> RandomWeapons;

    UFUNCTION(BlueprintCallable, Category = "WeaponBox")
    void OpenWeaponBox(APlayerCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "WeaponBox")
    void GenerateRandomWeapons();

protected:
    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnOverlapEnd(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );
};
