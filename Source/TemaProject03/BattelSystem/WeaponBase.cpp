#include "WeaponBase.h"
#include "Weapon/BazookaProjectile.h"
#include "TemaProject03/Enemy/EnemyCharacter.h"
#include "TemaProject03/Player/PController.h"

AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 무기 기준점 생성
    WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
    RootComponent = WeaponRoot;

    // 무기 메쉬 생성
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(WeaponRoot);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 스켈레탈 메시 총기 생성
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(WeaponRoot);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Weapon BeginPlay"));

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

    if (WeaponTable)
    {
        FWeaponData* Data = WeaponTable->FindRow<FWeaponData>(WeaponRowName, "");

        if (Data)
        {
            WeaponData = *Data;

            APlayerCharacter* Char = Cast<APlayerCharacter>(GetOwner());

            if (Char && Char->CurrentEffect)
            {
                WeaponData.MagazineSize = Char->CurrentEffect->ModifyMagazineSize(WeaponData.MagazineSize);
            }
            else if (CurrentEffect)
            {
                WeaponData.MagazineSize = CurrentEffect->ModifyMagazineSize(WeaponData.MagazineSize);
            }

            MaxAmmo = WeaponData.MagazineSize;
            CurrentAmmo = WeaponData.MagazineSize;

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

FWeaponData AWeaponBase::GetWeaponData() const
{
    if (WeaponTable)
    {
        if (FWeaponData* Data =
            WeaponTable->FindRow<FWeaponData>(WeaponRowName, ""))
        {
            return *Data;
        }
    }

    return FWeaponData();
}

UMeshComponent* AWeaponBase::GetActiveWeaponMesh() const
{
    if (SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset() && SkeletalMesh->IsVisible())
    {
        return SkeletalMesh;
    }

    return Mesh;
}

FVector AWeaponBase::GetMuzzleLocation() const
{
    if (UMeshComponent* ActiveMesh = GetActiveWeaponMesh())
    {
        return ActiveMesh->GetSocketLocation(TEXT("Muzzle"));
    }

    return GetActorLocation();
}

FRotator AWeaponBase::GetMuzzleRotation() const
{
    if (UMeshComponent* ActiveMesh = GetActiveWeaponMesh())
    {
        return ActiveMesh->GetSocketRotation(TEXT("Muzzle"));
    }

    return GetActorRotation();
}

void AWeaponBase::Fire()
{
    if (bIsReloading)
    {
        UE_LOG(LogTemp, Warning, TEXT("Reloading..."));
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

    if (SkeletalMesh && FireWeaponAnimation)
    {
        SkeletalMesh->PlayAnimation(FireWeaponAnimation, false);
    }

    if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
    {
        Player->PlayArmsMontage(ArmsFireMontage);
        Player->ApplyWeaponRecoil(WeaponData.RecoilPitch, WeaponData.RecoilYaw);
    }

    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Ammo();
    }

    APlayerCharacter* Char = Cast<APlayerCharacter>(GetOwner());

    float CharacterAtk = 0.f;
    if (Char)
    {
        CharacterAtk = Char->CharacterAttack;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AWeaponBase::ResetFire, WeaponData.FireRate, false);
        return;
    }

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PC)
    {
        GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AWeaponBase::ResetFire, WeaponData.FireRate, false);
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector Start = CameraLocation;
    FVector End = Start + (CameraRotation.Vector() * 10000.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwnerPawn);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    FVector MuzzleLocation = GetMuzzleLocation();
    FRotator MuzzleRotation = GetMuzzleRotation();

    /*DrawDebugLine(GetWorld(), Start, bHit ? Hit.Location : End, FColor::Red, false, 1.f);
    DrawDebugLine(GetWorld(), MuzzleLocation, bHit ? Hit.Location : End, FColor::Yellow, false, 1.f);
    DrawDebugSphere(GetWorld(), MuzzleLocation, 5.f, 12, FColor::Blue, false, 1.f);*/

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleLocation);
    }

    if (FireEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireEffect, MuzzleLocation, MuzzleRotation);
    }

    if (bHit)
    {
        //DrawDebugSphere(GetWorld(), Hit.Location, 20.f, 16, FColor::Green, false, 2.f);

        UE_LOG(LogTemp, Warning, TEXT("Hit Location: %s"), *Hit.Location.ToString());
        UE_LOG(LogTemp, Warning, TEXT("HIT START"));

        AActor* HitActor = Hit.GetActor();

        if (!HitActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("NO HIT ACTOR"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HIT ACTOR: %s"), *HitActor->GetName());
            UE_LOG(LogTemp, Warning, TEXT("Hit Bone: %s"), *Hit.BoneName.ToString());

            AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor);

            if (Enemy)
            {
                float Attack = CharacterAtk + WeaponData.Damage;
                float Damage = Attack - Enemy->Defense;
                float Distance = FVector::Dist(Start, Hit.Location);

                UWeaponEffectBase* ActiveEffect = CurrentEffect;
                if (Char && Char->CurrentEffect)
                {
                    ActiveEffect = Char->CurrentEffect;
                }

                UHeadHunterEffect* HeadHunter = Cast<UHeadHunterEffect>(ActiveEffect);
                UConfidenceEffect* Confidence = Cast<UConfidenceEffect>(ActiveEffect);

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
                    if (Hit.BoneName == TEXT("head"))
                    {
                        HeadHunter->OnHeadShot();
                        UE_LOG(LogTemp, Warning, TEXT("HEADSHOT"));
                    }
                }

                if (ActiveEffect)
                {
                    Damage = ActiveEffect->ModifyDamage(Damage, Distance, CurrentAmmo, WeaponData.MagazineSize);
                }

                Enemy->ApplyDamage(Damage);

                if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
                {
                    PlayerController->TriggerUICustomEvent(FName("ShowHitMarker"));
                }

                UE_LOG(LogTemp, Warning, TEXT("Damage: %f"), Damage);
            }
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

    // 총기 자체 장전 애니메이션
    if (SkeletalMesh && ReloadWeaponAnimation)
    {
        SkeletalMesh->PlayAnimation(ReloadWeaponAnimation, false);
    }

    // 팔 장전 애니메이션
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
    {
        Player->PlayArmsMontage(ArmsReloadMontage);
    }

    // 장전 사운드
    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());
    }

    // 재장전 시 ReloadBar 출력
    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->StartReloadUI();
    }

    UE_LOG(LogTemp, Warning, TEXT("Start Reload"));

    GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &AWeaponBase::FinishReload, WeaponData.ReloadTime, false);
}

void AWeaponBase::FinishReload()
{
    bIsReloading = false;

    CurrentAmmo = WeaponData.MagazineSize;

    bCanFire = true;

    // 재장전 후 탄약 HUD 업데이트
    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Ammo();
    }

    UE_LOG(LogTemp, Warning, TEXT("Reload Complete"));
}

void AWeaponBase::InitWeapon(APlayerCharacter* InOwner)
{
    OwnerCharacter = InOwner;
    SetOwner(InOwner);

    bCanFire = true;
    bIsReloading = false;

    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

    UE_LOG(LogTemp, Warning, TEXT("Weapon Init OK"));
}

int32 AWeaponBase::GetMaxAmmo() const
{
    return MaxAmmo;
}

int32 AWeaponBase::GetCurrentAmmo() const
{
    return CurrentAmmo;
}

void AWeaponBase::ResetFire()
{
    bCanFire = true;
}

void AWeaponBase::ApplyWeaponAttachTransform()
{
    // 총기 위치/회전/크기는 DT가 아니라 BP 또는 손 소켓에서 조정
    SetActorRelativeLocation(FVector::ZeroVector);
    SetActorRelativeRotation(FRotator::ZeroRotator);
    SetActorScale3D(FVector(1.0f));
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
    if (bIsReloading) return;

    if (CurrentAmmo <= 0)
    {
        Reload();
        return;
    }

    if (!bCanFire) return;

    bCanFire = false;
    CurrentAmmo--;

    if (SkeletalMesh && FireWeaponAnimation) SkeletalMesh->PlayAnimation(FireWeaponAnimation, false);

    if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
    {
        Player->PlayArmsMontage(ArmsFireMontage);
        Player->ApplyWeaponRecoil(WeaponData.RecoilPitch, WeaponData.RecoilYaw);
    }

    if (FireSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());

    if (FireEffect) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireEffect, GetMuzzleLocation(), GetMuzzleRotation());

    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
        PlayerController->UpdateHUD_Ammo();

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PC) return;

    FVector CameraLocation;
    FRotator CameraRotation;

    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector Start = CameraLocation;

    for (int32 i = 0; i < 8; i++)
    {
        float RandomYaw = FMath::RandRange(-6.f, 6.f);
        float RandomPitch = FMath::RandRange(-6.f, 6.f);

        FRotator SpreadRotation = CameraRotation;
        SpreadRotation.Yaw += RandomYaw;
        SpreadRotation.Pitch += RandomPitch;

        FVector End = Start + (SpreadRotation.Vector() * 10000.f);

        FHitResult Hit;

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(OwnerPawn);

        bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

        //DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 1.f);

        if (bHit)
        {
            AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Hit.GetActor());

            if (Enemy)
            {
                float Damage = WeaponData.Damage;

                Enemy->ApplyDamage(Damage);

                UE_LOG(LogTemp, Warning, TEXT("Shotgun Hit"));
            }
            
        }
    }
    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->TriggerUICustomEvent(FName("ShowHitMarker"));
    }
    GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AWeaponBase::ResetFire, WeaponData.FireRate, false);
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

    // 총기 자체 발사 애니메이션
    if (SkeletalMesh && FireWeaponAnimation)
    {
        SkeletalMesh->PlayAnimation(FireWeaponAnimation, false);
    }

    // 팔 발사 애니메이션 + 카메라 반동
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner()))
    {
        Player->PlayArmsMontage(ArmsFireMontage);
        Player->ApplyWeaponRecoil(WeaponData.RecoilPitch, WeaponData.RecoilYaw);
    }

    // Projectile 없으면 종료
    if (!BazookaProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BazookaProjectileClass is NULL"));

        GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AWeaponBase::ResetFire, WeaponData.FireRate, false);
        return;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());

    if (!OwnerPawn)
    {
        GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AWeaponBase::ResetFire, WeaponData.FireRate, false);
        return;
    }

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());

    if (!PC)
    {
        GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AWeaponBase::ResetFire, WeaponData.FireRate, false);
        return;
    }

    // 카메라 위치/회전
    FVector CameraLocation;
    FRotator CameraRotation;

    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // 화면 중앙 라인트레이스
    FVector TraceStart = CameraLocation;
    FVector TraceEnd = TraceStart + (CameraRotation.Vector() * 10000.f);

    FHitResult Hit;

    FCollisionQueryParams Params;

    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwnerPawn);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

    // 목표 위치
    FVector TargetPoint = bHit ? Hit.Location : TraceEnd;

    // 총구 위치
    FVector SpawnLocation = GetMuzzleLocation();

    // 목표 방향 계산
    FVector ShootDirection = (TargetPoint - SpawnLocation).GetSafeNormal();

    FRotator SpawnRotation = ShootDirection.Rotation();

    // Projectile 생성
    ABazookaProjectile* Projectile = GetWorld()->SpawnActor<ABazookaProjectile>(BazookaProjectileClass, SpawnLocation, SpawnRotation);

    // 생성 성공
    if (Projectile)
    {
        Projectile->SetDamage(WeaponData.Damage);

        Projectile->SetProjectileMesh(nullptr);

        Projectile->SetOwner(GetOwner());

        Projectile->Collision->IgnoreActorWhenMoving(GetOwner(), true);

        UE_LOG(LogTemp, Warning, TEXT("Bazooka Fired"));
    }

    GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AWeaponBase::ResetFire, WeaponData.FireRate, false);
}
