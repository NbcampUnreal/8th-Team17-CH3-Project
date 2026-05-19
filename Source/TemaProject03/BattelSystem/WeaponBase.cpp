#include "WeaponBase.h"
#include "Weapon/BazookaProjectile.h"
#include "TemaProject03/Enemy/EnemyCharacter.h"
#include "TemaProject03/Player/PlayerCharacter.h"
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

            MaxAmmo = WeaponData.MagazineSize;
            CurrentAmmo = WeaponData.MagazineSize;

            if (WeaponData.WeaponMesh)
            {
                Mesh->SetStaticMesh(WeaponData.WeaponMesh);
            }

            Mesh->SetRelativeLocation(WeaponData.MeshRelativeLocation);
            Mesh->SetRelativeRotation(WeaponData.MeshRelativeRotation);
            Mesh->SetRelativeScale3D(WeaponData.MeshRelativeScale);
            UE_LOG(LogTemp, Warning, TEXT("Weapon Camera Feedback Ready!"));

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

    // 카메라에 붙은 총기 자체를 움직이는 발사 연출
    PlayFireFeedback();

    // 탄약 감소된 후 HUD 업데이트
    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Ammo();
    }

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
            // Hit 이펙트 출력
            if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
            {
                PlayerController->TriggerUICustomEvent(FName("ShowHitMarker"));
            }

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

    // 재장전 시 ReloadAnim 출력
    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Reload(true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Start Reload"));

    GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &AWeaponBase::FinishReload, WeaponData.ReloadTime, false);
}

void AWeaponBase::FinishReload()
{
    bIsReloading = false;

    CurrentAmmo = WeaponData.MagazineSize;

    bCanFire = true;

    // 재장전 후 ReloadAnim 숨기기
    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Reload(false);
    }

    // 재장전 후 탄약 HUD 업데이트
    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Ammo();
    }

    UE_LOG(LogTemp, Warning, TEXT("Reload Complete"));
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
    // WeaponData에 저장된 총기별 카메라 부착값 적용
    SetActorRelativeLocation(WeaponData.AttachLocation);
    SetActorRelativeRotation(WeaponData.AttachRotation);
    SetActorScale3D(WeaponData.AttachScale);

    // 발사 반동 후 돌아올 기본 위치/회전 저장
    BaseRelativeLocation = WeaponData.AttachLocation;
    BaseRelativeRotation = WeaponData.AttachRotation;
}

void AWeaponBase::PlayFireFeedback()
{
    if (!GetWorld())
    {
        return;
    }

    // 이전 반동 복구 타이머가 남아있으면 정리
    GetWorld()->GetTimerManager().ClearTimer(FireFeedbackTimerHandle);

    // 카메라에 붙은 무기 자체를 살짝 움직여 발사 느낌을 만듦
    SetActorRelativeLocation(BaseRelativeLocation + WeaponData.FireRecoilLocationOffset);
    SetActorRelativeRotation(BaseRelativeRotation + WeaponData.FireRecoilRotationOffset);

    GetWorld()->GetTimerManager().SetTimer(
        FireFeedbackTimerHandle,
        this,
        &AWeaponBase::ResetWeaponFeedback,
        WeaponData.FireRecoilReturnTime,
        false
    );
}

void AWeaponBase::ResetWeaponFeedback()
{
    // 발사 연출 후 원래 위치/회전으로 복구
    SetActorRelativeLocation(BaseRelativeLocation);
    SetActorRelativeRotation(BaseRelativeRotation);
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

    // 카메라에 붙은 총기 자체를 움직이는 발사 연출
    PlayFireFeedback();

    APawn* OwnerPawn = Cast<APawn>(GetOwner());

    if (!OwnerPawn)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());

    if (!PC)
    {
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;

    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector Start = CameraLocation;

    for (int32 i = 0; i < 8; i++)
    {
        // 퍼짐값
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

        DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 1.f);

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

    // 카메라에 붙은 총기 자체를 움직이는 발사 연출
    PlayFireFeedback();

    // Projectile 없으면 종료
    if (!BazookaProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BazookaProjectileClass is NULL"));
        return;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());

    if (!OwnerPawn)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());

    if (!PC)
    {
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
    FVector SpawnLocation = Mesh->GetSocketLocation(TEXT("Muzzle"));

    // 목표 방향 계산
    FVector ShootDirection = (TargetPoint - SpawnLocation).GetSafeNormal();

    FRotator SpawnRotation = ShootDirection.Rotation();

    // Projectile 생성
    ABazookaProjectile* Projectile = GetWorld()->SpawnActor<ABazookaProjectile>(BazookaProjectileClass, SpawnLocation, SpawnRotation);

    // 생성 성공 시
    if (Projectile)
    {
        Projectile->SetDamage(WeaponData.Damage);

        Projectile->SetProjectileMesh(WeaponData.ProjectileMesh);

        Projectile->SetOwner(GetOwner());

        Projectile->Collision->IgnoreActorWhenMoving(GetOwner(), true);

        UE_LOG(LogTemp, Warning, TEXT("Bazooka Fired"));
    }

    GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &AWeaponBase::ResetFire, WeaponData.FireRate, false);
}
