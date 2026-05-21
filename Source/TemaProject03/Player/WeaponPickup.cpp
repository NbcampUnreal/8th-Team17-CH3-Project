#include "WeaponPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacter.h"
#include "TemaProject03/BattelSystem/WeaponBase.h"

AWeaponPickup::AWeaponPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
    RootComponent = PickupCollision;

    PickupCollision->SetSphereRadius(120.0f);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
    PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
    PreviewMesh->SetupAttachment(RootComponent);
    PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PreviewSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewSkeletalMesh"));
    PreviewSkeletalMesh->SetupAttachment(RootComponent);
    PreviewSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponPickup::BeginPlay()
{
    Super::BeginPlay();

    PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponPickup::OnPickupBeginOverlap);
    PickupCollision->OnComponentEndOverlap.AddDynamic(this, &AWeaponPickup::OnPickupEndOverlap);
}

TSubclassOf<AWeaponBase> AWeaponPickup::GetWeaponClass() const
{
    return WeaponClass;
}

void AWeaponPickup::SetWeaponClass(TSubclassOf<AWeaponBase> NewWeaponClass)
{
    WeaponClass = NewWeaponClass;
}

void AWeaponPickup::OnPickupBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    Player->SetNearbyWeaponPickup(this);

    UE_LOG(LogTemp, Warning, TEXT("[Pickup] Player can pick up weapon"));
}

void AWeaponPickup::OnPickupEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    Player->SetNearbyWeaponPickup(nullptr);

    UE_LOG(LogTemp, Warning, TEXT("[Pickup] Player left pickup range"));
}
