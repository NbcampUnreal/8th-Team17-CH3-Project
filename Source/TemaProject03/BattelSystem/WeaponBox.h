#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "WeaponBase.h"
#include "WeaponBox.generated.h"

class UBoxComponent;
class APlayerCharacter;

UCLASS()
class TEMAPROJECT03_API AWeaponBox : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AWeaponBox();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere)
    class UBoxComponent* BoxCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<AWeaponBase>> WeaponPool;

    UPROPERTY(BlueprintReadOnly)
    TArray<AWeaponBase*> RandomWeapons;

protected:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* BoxMesh;
public:
    UFUNCTION(BlueprintCallable)
    void GenerateRandomWeapons();
    void OpenWeaponBox(APlayerCharacter* Player);
};
