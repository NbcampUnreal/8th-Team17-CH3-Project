#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPGProjectile.generated.h"

UCLASS()
class TEMAPROJECT03_API ARPGProjectile : public AActor
{
    GENERATED_BODY()

public:
    ARPGProjectile();

protected:
    virtual void BeginPlay() override;

    // 충돌 감지용 Sphere
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USphereComponent* CollisionComp;

    // 투사체 이동 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    class UProjectileMovementComponent* ProjectileMovement;

    // 메시
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* MeshComp;

    // 충돌 이벤트
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);

    // 중복 폭발 방지
    bool bHasExploded = false;

public:
    // 스킬에서 계산된 데미지, 폭발 범위, 공격 횟수를 받아옴
    void InitProjectile(float InDamage, float InExplosionRadius, int32 InHitCount);

    // 스킬 파일에 적힌 속도 값을 받아와서 적용할 함수
    void SetProjectileSpeed(float NewSpeed);

    // 최대 사거리와 속도를 받아 LifeSpan 계산에 사용
    void SetFireRange(float InFireRange, float InSpeed);

private:
    // 충돌 시 폭발 처리
    void Explode();

    // 사거리 / 속도 = 생존 시간 계산용
    float FireRange = 10000.0f;
    float ProjectileSpeed = 3000.0f;

    // 실제 데미지는 Skill_Soilder에서 계산
    float DamageAmount = 100.0f;

    // 폭발 반지름
    float ExplosionRadius = 350.0f;

    // 폭발 시 공격 횟수
    int32 HitCount = 3;
};
