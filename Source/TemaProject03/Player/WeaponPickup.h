#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UPrimitiveComponent;
class AWeaponBase;
class APlayerCharacter;

UCLASS()
class TEMAPROJECT03_API AWeaponPickup : public AActor
{
    GENERATED_BODY()

public:
    AWeaponPickup();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
    USphereComponent* PickupCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
    UStaticMeshComponent* PreviewMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
    USkeletalMeshComponent* PreviewSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    TSubclassOf<AWeaponBase> WeaponClass;

    TSubclassOf<AWeaponBase> GetWeaponClass() const;
    void SetWeaponClass(TSubclassOf<AWeaponBase> NewWeaponClass);

private:
    UFUNCTION()
    void OnPickupBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnPickupEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );
};
