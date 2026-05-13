#include "RPGProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "DrawDebugHelpers.h"
#include "TemaProject03/Enemy/EnemyCharacter.h"

ARPGProjectile::ARPGProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    // 충돌이 실제로 막히도록 Block 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);

    CollisionComp->OnComponentHit.AddDynamic(this, &ARPGProjectile::OnHit);

    RootComponent = CollisionComp;

    // 투사체 이동 컴포넌트 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;

    // 초기값은 0으로 두고 스킬에서 설정
    ProjectileMovement->InitialSpeed = 0.f;
    ProjectileMovement->MaxSpeed = 0.f;

    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    // 충돌 시 튕기지 않도록 설정
    ProjectileMovement->bShouldBounce = false;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);

    // 메시는 충돌에 관여하지 않음
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARPGProjectile::BeginPlay()
{
    Super::BeginPlay();

    // Owner와 충돌하지 않도록 무시
    if (AActor* OwnerActor = GetOwner())
    {
        CollisionComp->IgnoreActorWhenMoving(OwnerActor, true);
    }

    // 방어코드 속도가 0이면 LifeSpan 계산 불가
    if (ProjectileSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[RPGProjectile] ProjectileSpeed is 0! Check SetProjectileSpeed call."));
        return;
    }

    // 사거리 / 속도 = 투사체 생존 시간
    float LifeTime = FireRange / ProjectileSpeed;

    UE_LOG(LogTemp, Warning,
        TEXT("[RPGProjectile] Spawned. Speed: %.1f, Range: %.1f, LifeTime: %.2f sec"),
        ProjectileSpeed,
        FireRange,
        LifeTime);

    // 일정 시간 후 자동 Destroy
    // LifeSpan으로 사라질 때는 폭발하지 않음
    SetLifeSpan(LifeTime);
}

void ARPGProjectile::InitProjectile(float InDamage, float InExplosionRadius, int32 InHitCount)
{
    DamageAmount = InDamage;
    ExplosionRadius = InExplosionRadius;
    HitCount = InHitCount;
}

void ARPGProjectile::SetProjectileSpeed(float NewSpeed)
{
    // 방어코드 유효한 속도 값인지 확인
    if (NewSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[RPGProjectile] Invalid Speed: %.1f"),
            NewSpeed);

        return;
    }

    ProjectileSpeed = NewSpeed;

    if (ProjectileMovement)
    {
        ProjectileMovement->InitialSpeed = NewSpeed;
        ProjectileMovement->MaxSpeed = NewSpeed;

        ProjectileMovement->Velocity =
            GetActorForwardVector() * NewSpeed;
    }
}

void ARPGProjectile::SetFireRange(float InFireRange, float InSpeed)
{
    // 방어코드 유효한 값인지 확인
    if (InFireRange <= 0.0f || InSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[RPGProjectile] Invalid FireRange or Speed!"));

        return;
    }

    FireRange = InFireRange;
    ProjectileSpeed = InSpeed;

    // 스폰 이후 스킬에서 값이 들어오므로 LifeSpan을 다시 계산
    SetLifeSpan(FireRange / ProjectileSpeed);
}

void ARPGProjectile::OnHit(UPrimitiveComponent* HitComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    // 방어코드 이미 폭발했으면 중복 처리 방지
    if (bHasExploded)
        return;

    // 방어코드 자기 자신 또는 Owner와 충돌 무시
    if (!OtherActor ||
        OtherActor == this ||
        OtherActor == GetOwner())
    {
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[RPGProjectile] Hit: %s"),
        *OtherActor->GetName());

    // 충돌 시점에서만 폭발
    Explode();

    // 폭발 처리 후 제거
    Destroy();
}

void ARPGProjectile::Explode()
{
    // 방어코드 이미 폭발했으면 중복 처리 방지
    if (bHasExploded)
        return;

    if (!GetWorld())
        return;

    bHasExploded = true;

    UE_LOG(LogTemp, Warning,
        TEXT("[RPGProjectile] Explode! Location: %s, Radius: %.1f"),
        *GetActorLocation().ToString(),
        ExplosionRadius);

    // 디버그: 주황색 구체 시각화
    DrawDebugSphere(GetWorld(), GetActorLocation(),
        ExplosionRadius, 16, FColor::Orange, false, 2.0f);

    TArray<AActor*> OverlappedActors;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    if (AActor* OwnerActor = GetOwner())
    {
        ActorsToIgnore.Add(OwnerActor);
    }

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        ExplosionRadius,
        ObjectTypes,
        AEnemyCharacter::StaticClass(),
        ActorsToIgnore,
        OverlappedActors
    );

    if (OverlappedActors.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RPGProjectile] No enemies in explosion range."));
        return;
    }

    // 같은 몬스터가 여러 컴포넌트로 잡혀도 한 번만 처리
    TSet<AEnemyCharacter*> DamagedEnemies;

    for (AActor* Actor : OverlappedActors)
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Actor);
        if (!Enemy || DamagedEnemies.Contains(Enemy))
        {
            continue;
        }

        DamagedEnemies.Add(Enemy);

        UE_LOG(LogTemp, Warning, TEXT("[RPGProjectile] Enemy in range: %s"), *Enemy->GetName());

        for (int32 i = 0; i < HitCount; i++)
        {
            Enemy->ApplyDamage(DamageAmount);
            UE_LOG(LogTemp, Warning, TEXT("[RPGProjectile] Attack %d/%d -> Damage: %.1f"),
                i + 1, HitCount, DamageAmount);
        }
    }
}
