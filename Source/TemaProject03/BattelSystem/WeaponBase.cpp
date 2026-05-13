#include "WeaponBase.h"
#include "Weapon/BazookaProjectile.h"
#include "TemaProject03/Enemy/EnemyCharacter.h"
#include "TemaProject03/Player/PlayerCharacter.h"


AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 무기 메쉬 생성
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

            if (CurrentEffect)
            {
                WeaponData.MagazineSize =
                    CurrentEffect->ModifyMagazineSize(
                        WeaponData.MagazineSize
                    );
            }

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

    if (EffectClasses.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, EffectClasses.Num() - 1);

        TSubclassOf<UWeaponEffectBase> RandomEffect = EffectClasses[RandomIndex];

        CurrentEffect = NewObject<UWeaponEffectBase>(this, RandomEffect);

        if (CurrentEffect)
        {
            UE_LOG(LogTemp, Warning, TEXT("Current Effect: %s"), *CurrentEffect->GetClass()->GetName());
        }
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

    if (!bCanFire)
    {
        return;
    }

    bCanFire = false;

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
    FVector End = Start + (CameraRotation.Vector() * 10000.f);

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
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            MuzzleFlash,
            MuzzleLocation,
            MuzzleRotation
        );
    }

    // 히트 처리
    if (bHit)
    {
        DrawDebugSphere(GetWorld(), Hit.Location, 20.f, 16, FColor::Green, false, 2.f);

        UE_LOG(LogTemp, Warning, TEXT("Hit Location: %s"), *Hit.Location.ToString());

        UE_LOG(LogTemp, Warning, TEXT("HIT START"));

        AActor* HitActor = Hit.GetActor();

        if (!HitActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("NO HIT ACTOR"));
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("HIT ACTOR: %s"), *HitActor->GetName());

        UE_LOG(LogTemp, Warning, TEXT("Hit Bone: %s"), *Hit.BoneName.ToString());

        // 적 캐릭터 캐스팅
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor);

        if (Enemy)
        {
            // 공격력 계산 (캐릭터 + 무기)
            float Attack = CharacterAtk + WeaponData.Damage;

            // 방어력 적용
            float Damage = Attack - Enemy->Defense;

            float Distance = FVector::Dist(Start, Hit.Location);

            UHeadHunterEffect* HeadHunter = Cast<UHeadHunterEffect>(CurrentEffect);

            UConfidenceEffect* Confidence = Cast<UConfidenceEffect>(CurrentEffect);

            if (Confidence)
            {
                if (Hit.BoneName == TEXT("head"))
                {
                    Confidence->OnHeadShot();

                    UE_LOG(LogTemp, Warning, TEXT("SELF HEADSHOT"));
                }
                else
                {
                    Confidence->OnBodyShot();

                    UE_LOG(LogTemp, Warning, TEXT("SELF BODYSHOT"));
                }
            }

            if (HeadHunter)
            {
                UE_LOG(LogTemp, Warning, TEXT("Hit Bone: %s"), *Hit.BoneName.ToString());

                // head 본 맞췄는지 확인
                if (Hit.BoneName == TEXT("head"))
                {
                    HeadHunter->OnHeadShot();

                    UE_LOG(LogTemp, Warning, TEXT("HEADSHOT"));
                }
            }

            if (CurrentEffect)
            {
                Damage = CurrentEffect->ModifyDamage(Damage, Distance, CurrentAmmo, WeaponData.MagazineSize);
            }


            // 데미지 적용
            Enemy->ApplyDamage(Damage);

            UE_LOG(LogTemp, Warning, TEXT("Damage: %f"), Damage);
        }
    }

    GetWorld()->GetTimerManager().SetTimer(
        FireRateTimerHandle,
        this,
        &AWeaponBase::ResetFire,
        WeaponData.FireRate,
        false
    );
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

    bCanFire = true;

    UE_LOG(LogTemp, Warning, TEXT("Reload Complete"));
}

int32 AWeaponBase::GetCurrentAmmo() const
{
    return CurrentAmmo;
}

void AWeaponBase::ResetFire()
{
    bCanFire = true;
}

void AWeaponBase::StartFire()
{
    switch (WeaponData.FireType)
    {
    case EFireType::Single:

        Fire();
        break;

    case EFireType::Auto:

        Fire();

        GetWorld()->GetTimerManager().SetTimer(AutoFireTimerHandle, this, &AWeaponBase::Fire, WeaponData.FireRate, true);

        break;

    case EFireType::Shotgun:

        FireShotgun();
        break;

    case EFireType::Bow:

        bIsChargingBow = true;

        BowChargeStartTime = GetWorld()->GetTimeSeconds();

        UE_LOG(LogTemp, Warning, TEXT("Bow Charging Start"));

        break;

    case EFireType::Bazooka:

        FireBazooka();
        break;
    }
}

void AWeaponBase::StopFire()
{
    GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);

    if (WeaponData.FireType == EFireType::Bow)
    {
        if (bIsChargingBow)
        {
            float ChargeTime =
                GetWorld()->GetTimeSeconds() - BowChargeStartTime;

            UE_LOG(LogTemp, Warning, TEXT("Charge Time: %f"), ChargeTime);

            // 충분히 차징했는지 확인
            if (ChargeTime >= BowChargeTime)
            {
                FireBow();

                UE_LOG(LogTemp, Warning, TEXT("Bow Fired"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Bow Charge Failed"));
            }

            bIsChargingBow = false;
        }
    }
}

void AWeaponBase::FireShotgun()
{
    for (int32 i = 0; i < 8; i++)
    {
        Fire();
    }
}

void AWeaponBase::FireBow()
{
    Fire();
}

void AWeaponBase::FireBazooka()
{
    if (bIsReloading)
    {
        return;
    }

    if (CurrentAmmo <= 0)
    {
        Reload();
        return;
    }

    if (!bCanFire)
    {
        return;
    }

    bCanFire = false;

    CurrentAmmo--;

    // Projectile 없으면 종료
    if (!BazookaProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BazookaProjectileClass is NULL"));
        return;
    }

    // 총구 위치
    FVector SpawnLocation = Mesh->GetSocketLocation(TEXT("Muzzle"));

    // 총구 회전
    FRotator SpawnRotation = Mesh->GetSocketRotation(TEXT("Muzzle"));

    // Projectile 생성
    ABazookaProjectile* Projectile =
        GetWorld()->SpawnActor<ABazookaProjectile>(BazookaProjectileClass, SpawnLocation, SpawnRotation);

    // 생성 성공 시
    if (Projectile)
    {
        Projectile->SetDamage(WeaponData.Damage);
        Projectile->SetProjectileMesh(WeaponData.ProjectileMesh);

        Projectile->SetOwner(GetOwner());
        Projectile->Collision->IgnoreActorWhenMoving(GetOwner(), true);

        UE_LOG(LogTemp, Warning, TEXT("Bazooka Fired"));
    }
    GetWorld()->GetTimerManager().SetTimer(
        FireRateTimerHandle,
        this,
        &AWeaponBase::ResetFire,
        WeaponData.FireRate,
        false
    );
}
