#include "ProjectileBase.h"
#include "Kismet/GameplayStatics.h"

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

    ProjectileMovement->InitialSpeed = 3000.f;

    ProjectileMovement->MaxSpeed = 3000.f;

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
    // 범위 폭발
    if (bUseRadialDamage)
    {
        UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage, GetActorLocation(), ExplosionRadius, nullptr, TArray<AActor*>(), this, nullptr, true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Projectile Hit"));

    Destroy();
}

void AProjectileBase::SetProjectileMesh(UStaticMesh* NewMesh)
{
    if (NewMesh)
    {
        Mesh->SetStaticMesh(NewMesh);
    }
}
