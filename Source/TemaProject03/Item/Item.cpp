#include "TemaProject03/Item/Item.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TemaProject03/Player/PlayerCharacter.h"
#include "TemaProject03/BattelSystem/WeaponBase.h"

AItem::AItem()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;

    CollisionSphere->SetSphereRadius(80.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionSphere->SetGenerateOverlapEvents(true);

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItem::BeginPlay()
{
    Super::BeginPlay();

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnItemOverlap);
}

void AItem::OnItemOverlap(
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

    UseItem(Player);

    if (bDestroyOnPickup)
    {
        Destroy();
    }
}

void AItem::UseItem_Implementation(APlayerCharacter* Player)
{
}

void AHealthRecoveryItem::UseItem_Implementation(APlayerCharacter* Player)
{
    if (!Player)
    {
        return;
    }

    Player->RestoreHealthByPercent(RecoveryPercent);
}

void AWeaponItem::UseItem_Implementation(APlayerCharacter* Player)
{
    if (!Player || !WeaponClass)
    {
        return;
    }

    //Player->EquipWeapon(WeaponClass);
}
