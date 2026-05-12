#include "BazookaProjectile.h"

ABazookaProjectile::ABazookaProjectile()
{
    // 범위 폭발 사용
    bUseRadialDamage = true;

    // 폭발 범위
    ExplosionRadius = 500.f;

    // 로켓 속도
    ProjectileMovement->InitialSpeed = 2000.f;

    ProjectileMovement->MaxSpeed = 2000.f;

    UE_LOG(LogTemp, Warning, TEXT("Bazooka Projectile Created"));
}
