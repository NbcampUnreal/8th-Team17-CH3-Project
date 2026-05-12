#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "ProjectileBase.generated.h"

UCLASS()
class TEMAPROJECT03_API AProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    AProjectileBase();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere)
    USphereComponent* Collision;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    void SetProjectileMesh(UStaticMesh* NewMesh);

    UPROPERTY(VisibleAnywhere)
    UProjectileMovementComponent* ProjectileMovement;

    // 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 30.f;

    // 폭발 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExplosionRadius = 300.f;

    // 폭발 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseRadialDamage = false;

    // 데미지 설정 함수
    void SetDamage(float NewDamage);

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
