#include "ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "TemaProject03/Enemy/EnemyCharacter.h"
#include "DrawDebugHelpers.h"

AProjectileBase::AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 충돌체 생성
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));

    RootComponent = Collision;

    Collision->SetSphereRadius(15.f);

    // 메쉬 생성
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

    Mesh->SetupAttachment(RootComponent);

    // 투사체 이동
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

    ProjectileMovement->InitialSpeed = 4000.f;

    ProjectileMovement->MaxSpeed = 4000.f;

    ProjectileMovement->ProjectileGravityScale = 0.1f;

    ProjectileMovement->bRotationFollowsVelocity = true;

    // 충돌 이벤트 연결
    Collision->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();
}

void AProjectileBase::SetDamage(float NewDamage)
{
    Damage = NewDamage;
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 자기 자신 무시
    if (OtherActor == GetOwner())
    {
        return;
    }

    FVector ExplosionLocation = GetActorLocation();

    // 폭발 범위 디버그
    DrawDebugSphere(GetWorld(), ExplosionLocation, ExplosionRadius, 32, FColor::Red, false, 2.f);

    // 모든 Enemy 찾기
    TArray<AActor*> EnemyActors;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), EnemyActors);

    for (AActor* Actor : EnemyActors)
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Actor);

        if (!Enemy)
        {
            continue;
        }

        float Distance = FVector::Dist(ExplosionLocation, Enemy->GetActorLocation());

        // 폭발 범위 안이면 데미지
        if (Distance <= ExplosionRadius)
        {
            Enemy->ApplyDamage(Damage);

            UE_LOG(LogTemp, Warning, TEXT("Explosion Hit"));
        }
    }

    Destroy();
}

void AProjectileBase::SetProjectileMesh(UStaticMesh* NewMesh)
{
    if (NewMesh)
    {
        Mesh->SetStaticMesh(NewMesh);
    }
}
