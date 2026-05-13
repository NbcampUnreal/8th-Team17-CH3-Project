#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class APlayerCharacter;
class AWeaponBase;

UCLASS()
class TEMAPROJECT03_API AItem : public AActor
{
    GENERATED_BODY()

public:
    AItem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bDestroyOnPickup = true;

    UFUNCTION()
    void OnItemOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void UseItem(APlayerCharacter* Player);

    virtual void UseItem_Implementation(APlayerCharacter* Player);
};

UCLASS()
class TEMAPROJECT03_API AHealthRecoveryItem : public AItem
{
    GENERATED_BODY()

public:
    virtual void UseItem_Implementation(APlayerCharacter* Player) override;
};

UCLASS()
class TEMAPROJECT03_API AWeaponItem : public AItem
{
    GENERATED_BODY()

public:
    virtual void UseItem_Implementation(APlayerCharacter* Player) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Weapon")
    TSubclassOf<AWeaponBase> WeaponClass;
};
