#include "WeaponBox.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TemaProject03/BattelSystem/WeaponBase.h"
#include "TemaProject03/Player/PlayerCharacter.h"

AWeaponBox::AWeaponBox()
{
    PrimaryActorTick.bCanEverTick = false;

    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    RootComponent = BoxCollision;

    BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BoxCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    BoxCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    BoxCollision->SetGenerateOverlapEvents(true);

    BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
    BoxMesh->SetupAttachment(RootComponent);
    BoxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBox::BeginPlay()
{
    Super::BeginPlay();

    BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBox::OnOverlapBegin);
    BoxCollision->OnComponentEndOverlap.AddDynamic(this, &AWeaponBox::OnOverlapEnd);
}

void AWeaponBox::GenerateRandomWeapons()
{
    for (AWeaponBase* Weapon : RandomWeapons)
    {
        if (Weapon)
        {
            Weapon->Destroy();
        }
    }

    RandomWeapons.Empty();

    if (!GetWorld() || WeaponPool.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] WeaponPool is empty."));
        return;
    }

    for (int32 i = 0; i < 3; i++)
    {
        const int32 RandomIndex = FMath::RandRange(0, WeaponPool.Num() - 1);
        TSubclassOf<AWeaponBase> WeaponClass = WeaponPool[RandomIndex];

        if (!WeaponClass)
        {
            continue;
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(
            WeaponClass,
            GetActorLocation(),
            GetActorRotation(),
            SpawnParams
        );

        if (!NewWeapon)
        {
            continue;
        }

        NewWeapon->SetActorHiddenInGame(true);
        NewWeapon->SetActorEnableCollision(false);

        RandomWeapons.Add(NewWeapon);

        UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] Random Weapon: %s"), *NewWeapon->GetName());
    }
}

void AWeaponBox::OpenWeaponBox(APlayerCharacter* Player)
{
    if (!Player)
    {
        return;
    }

    GenerateRandomWeapons();

    Player->CurrentRandomWeapons = RandomWeapons;

    UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] Opened / Weapon Count: %d"), RandomWeapons.Num());
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
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

    if (Player)
    {
        Player->SetNearbyWeaponBox(this);
        UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] Player Overlap Begin"));
    }
}

void AWeaponBox::OnOverlapEnd(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

    if (Player)
    {
        Player->SetNearbyWeaponBox(nullptr);
        UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] Player Overlap End"));
    }
}
