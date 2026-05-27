#include "WeaponBox.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AWeaponBox::AWeaponBox()
{
    PrimaryActorTick.bCanEverTick = false;

    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    RootComponent = BoxCollision;

    BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BoxCollision->SetGenerateOverlapEvents(false);

    BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
    BoxMesh->SetupAttachment(RootComponent);
    BoxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBox::BeginPlay()
{
    Super::BeginPlay();
}

void AWeaponBox::GenerateRandomWeapons()
{
    // WeaponBox 기능은 현재 사용하지 않음
}

void AWeaponBox::OpenWeaponBox(APlayerCharacter* Player)
{
    // WeaponBox 기능은 현재 사용하지 않음
}

void AWeaponBox::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    // WeaponBox 기능은 현재 사용하지 않음
}

void AWeaponBox::OnOverlapEnd(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    // WeaponBox 기능은 현재 사용하지 않음
}
