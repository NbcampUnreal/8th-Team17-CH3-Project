#include "TemaProject03/Player/PlayerCharacter.h"
#include "WeaponBox.h"

AWeaponBox::AWeaponBox()
{
    PrimaryActorTick.bCanEverTick = false;

    BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    RootComponent = BoxCollision;

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

void AWeaponBox::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AWeaponBox::GenerateRandomWeapons()
{
    RandomWeapons.Empty();

    if (WeaponPool.Num() <= 0)
    {
        return;
    }

    for (int32 i = 0; i < 3; i++)
    {
        int32 RandomIndex =
            FMath::RandRange(0, WeaponPool.Num() - 1);

        AWeaponBase* NewWeapon =
            GetWorld()->SpawnActor<AWeaponBase>(
                WeaponPool[RandomIndex]
            );

        if (NewWeapon)
        {
            // 처음엔 숨겨둠
            NewWeapon->SetActorHiddenInGame(true);

            // 충돌도 꺼두는게 안전
            NewWeapon->SetActorEnableCollision(false);

            RandomWeapons.Add(NewWeapon);

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Random Weapon: %s"),
                *NewWeapon->GetName()
            );
        }
    }
}

void AWeaponBox::OpenWeaponBox(APlayerCharacter* Player)
{
    GenerateRandomWeapons();

    if (!Player)
    {
        return;
    }

    Player->CurrentRandomWeapons = RandomWeapons;

    UE_LOG(LogTemp, Warning, TEXT("Weapon Box Opened"));
}

void AWeaponBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("BOX OVERLAP"));

    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

    if (Player)
    {
        Player->SetNearbyWeaponBox(this);
    }
}

void AWeaponBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

    if (Player)
    {
        Player->SetNearbyWeaponBox(nullptr);
    }
}
