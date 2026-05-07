#include "WeaponBase.h"
#include "TemaProject03/Enemy/EnemyCharacter.h"
#include "TemaProject03/Player/PlayerCharacter.h"


AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 무기 메쉬 생성
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Weapon BeginPlay"));

    if (WeaponTable)
    {
        FWeaponData* Data = WeaponTable->FindRow<FWeaponData>(WeaponRowName, "");

        if (Data)
        {
            WeaponData = *Data;

            CurrentAmmo = WeaponData.MagazineSize;

            if (WeaponData.WeaponMesh)
            {
                Mesh->SetStaticMesh(WeaponData.WeaponMesh);
            }

            UE_LOG(LogTemp, Warning, TEXT("Ammo Loaded: %d"), CurrentAmmo);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Weapon Row Not Found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponTable is NULL"));
    }
}

void AWeaponBase::Fire()
{
    // 리로드 중이면 발사 막기
    if (bIsReloading)
    {
        UE_LOG(LogTemp, Warning, TEXT("Reloading..."));
        return;
    }

    // 탄약 없으면 리로드
    if (CurrentAmmo <= 0)
    {
        Reload();
        return;
    }

    CurrentAmmo--; // 탄약 감소

    // 플레이어 캐릭터 가져오기
    APlayerCharacter* Char = Cast<APlayerCharacter>(GetOwner());

    float CharacterAtk = 0.f;
    if (Char)
    {
        CharacterAtk = Char->CharacterAttack; // 캐릭터 공격력
    }

    // 컨트롤러 가져오기
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PC) return;

    // 카메라 위치/방향 가져오기
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // 라인트레이스 시작/끝
    FVector Start = CameraLocation;
    FVector End = Start + (CameraRotation.Vector() * 1000.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwnerPawn);

    // 총알 판정
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    FVector TargetPoint = bHit ? Hit.Location : End;

    // 총구 위치
    FVector MuzzleLocation = Mesh->GetSocketLocation(TEXT("Muzzle"));
    FRotator MuzzleRotation = Mesh->GetSocketRotation(TEXT("Muzzle"));

    FVector ShootDirection = (TargetPoint - MuzzleLocation).GetSafeNormal();

    // 디버그 라인
    DrawDebugLine(GetWorld(), Start, TargetPoint, FColor::Red, false, 1.f);
    DrawDebugLine(GetWorld(), MuzzleLocation, TargetPoint, FColor::Yellow, false, 1.f);
    DrawDebugSphere(GetWorld(), MuzzleLocation, 5.f, 12, FColor::Blue, false, 1.f);

    // 발사 이펙트
    if (MuzzleFlash)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleLocation, MuzzleRotation);
    }

    // 히트 처리
    if (bHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("HIT START"));

        AActor* HitActor = Hit.GetActor();

        if (!HitActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("NO HIT ACTOR"));
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("HIT ACTOR: %s"), *HitActor->GetName());

        // 적 캐릭터 캐스팅
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor);

        if (Enemy)
        {
            // 공격력 계산 (캐릭터 + 무기)
            float Attack = CharacterAtk + WeaponData.Damage;

            // 방어력 적용
            float Damage = Attack - Enemy->Defense;

            // 최소/최대 데미지 제한
            Damage = FMath::Clamp(Damage, 29.f, 56.f);

            // 데미지 적용
            Enemy->ApplyDamage(Damage);

            UE_LOG(LogTemp, Warning, TEXT("Damage: %f"), Damage);
        }
    }
}
void AWeaponBase::Reload()
{
    if (bIsReloading) return;

    bIsReloading = true;

    UE_LOG(LogTemp, Warning, TEXT("Start Reload"));

    GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &AWeaponBase::FinishReload, WeaponData.ReloadTime, false);
}
void AWeaponBase::FinishReload()
{
    bIsReloading = false;

    CurrentAmmo = WeaponData.MagazineSize;

    UE_LOG(LogTemp, Warning, TEXT("Reload Complete"));
}

int32 AWeaponBase::GetCurrentAmmo() const
{
    return CurrentAmmo;
}
