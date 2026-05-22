#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"

#include "DrawDebugHelpers.h"
#include "TemaProject03/BattelSystem/WeaponBase.h"
#include "WeaponPickup.h"
#include "SkillComponent.h"
#include "PController.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    // 1인칭 카메라
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComp->SetupAttachment(RootComponent);

    SpringArmComp->TargetArmLength = 10.0f;
    SpringArmComp->SetRelativeLocation(FVector(0, 0, 60.0f));
    SpringArmComp->bUsePawnControlRotation = true;

    SpringArmComp->bDoCollisionTest = true;
    SpringArmComp->ProbeSize = 5.0f;

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComp->SetupAttachment(SpringArmComp);
    CameraComp->bUsePawnControlRotation = false;

    // 1인칭 팔 메시
    FirstPersonArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));
    FirstPersonArmsMesh->SetupAttachment(CameraComp);
    FirstPersonArmsMesh->SetOnlyOwnerSee(true);
    FirstPersonArmsMesh->bCastDynamicShadow = false;
    FirstPersonArmsMesh->CastShadow = false;

    // 스킬 컴포넌트 생성 (추가됨)
    SkillComp = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComp"));

    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input 적용
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // 카메라 피치 각도 제한
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->ViewPitchMin = MinViewPitch;
            PC->PlayerCameraManager->ViewPitchMax = MaxViewPitch;
        }
    }

    // 데이터 테이블에서 스탯 가져오기
    if (StatTable && !CharacterRowName.IsNone())
    {
        FCharacterStatRow* StatRow = StatTable->FindRow<FCharacterStatRow>(CharacterRowName, TEXT(""));

        if (StatRow)
        {
            MaxHealth = StatRow->Health;
            CurrentHealth = StatRow->Health;
            AttackDamage = StatRow->AttackDamage;
            Defense = StatRow->Defense;
            NormalSpeed = StatRow->MoveSpeed;

            UE_LOG(LogTemp, Warning, TEXT("Character Stat Loaded: %s"), *CharacterRowName.ToString());
            UE_LOG(LogTemp, Warning, TEXT("ID: %s | HP: %f | Speed: %f"), *CharacterRowName.ToString(), CurrentHealth, NormalSpeed);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DataTable Row '%s' not found!"), *CharacterRowName.ToString());
        }
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
    }

    // 무기 스폰 및 부착 로직 추가
    if (WeaponClass)
    {
        EquipWeapon(WeaponClass);
    }

    // 게임 시작 시 Ammo / SkillCooldown 업데이트
    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Ammo();

        if (SkillComp && SkillComp->CurrentSkill)
        {
            PlayerController->UpdateHUD_SkillCooldown(SkillComp->CurrentSkill->bIsOnCooldown);
        }
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

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInput->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerCharacter::Dash);

        // 발사
        EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &APlayerCharacter::StartFire);
        EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopFire);

        // 리로드
        EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &APlayerCharacter::StartReload);

        // 숫자키 무기 장착
        if (RifleAction)
        {
            EnhancedInput->BindAction(RifleAction, ETriggerEvent::Started, this, &APlayerCharacter::EquipRifle);
        }

        if (ShotgunAction)
        {
            EnhancedInput->BindAction(ShotgunAction, ETriggerEvent::Started, this, &APlayerCharacter::EquipShotgun);
        }

        if (PistolAction)
        {
            EnhancedInput->BindAction(PistolAction, ETriggerEvent::Started, this, &APlayerCharacter::EquipPistol);
        }

        // F키 상호작용
        if (InteractAction)
        {
            EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::Interact);
        }

        // 스킬 실행
        if (SkillAction)
        {
            EnhancedInput->BindAction(SkillAction, ETriggerEvent::Started, this, &APlayerCharacter::UseSkillInput);
        }
    }
}

float APlayerCharacter::GetOriginalSkillCooldown() const
{
    if (!SkillComp || !SkillComp->CurrentSkill)
    {
        return 0.0f;
    }

    return SkillComp->CurrentSkill->Cooldown;
}

bool APlayerCharacter::GetRPGMuzzleTransform(FTransform& OutMuzzleTransform) const
{
    if (!EquippedRPGActor)
    {
        return false;
    }

    TArray<USceneComponent*> SceneComponents;
    EquippedRPGActor->GetComponents<USceneComponent>(SceneComponents);

    for (USceneComponent* SceneComponent : SceneComponents)
    {
        if (SceneComponent && SceneComponent->GetName() == TEXT("Muzzle"))
        {
            OutMuzzleTransform = SceneComponent->GetComponentTransform();
            return true;
        }
    }

    // Muzzle 컴포넌트가 없으면 RPG 액터 앞쪽 위치를 임시 총구로 사용
    OutMuzzleTransform = EquippedRPGActor->GetActorTransform();
    OutMuzzleTransform.SetLocation(
        EquippedRPGActor->GetActorLocation() + EquippedRPGActor->GetActorForwardVector() * 100.0f
    );

    return true;
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddMovementInput(GetActorForwardVector(), MovementVector.X);
        AddMovementInput(GetActorRightVector(), MovementVector.Y);
    }
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

float APlayerCharacter::GetRemainingCooldown() const
{
    if (!bIsDashOnCooldown) return 0.0f;
    return GetWorld()->GetTimerManager().GetTimerRemaining(DashCooldownTimerHandle);
}

void APlayerCharacter::EquipWeapon()
{
    EquipWeapon(WeaponClass);
}

void APlayerCharacter::EquipRifle()
{
    EquipWeapon(RifleClass);
}

void APlayerCharacter::EquipShotgun()
{
    EquipWeapon(ShotgunClass);
}

void APlayerCharacter::EquipPistol()
{
    EquipWeapon(PistolClass);
}

void APlayerCharacter::ChangeWeapon(TSubclassOf<AWeaponBase> NewWeaponClass)
{
    EquipWeapon(NewWeaponClass);
}

// 대시 로직: 공중 대시 가능 버전 + 쿨타임 적용
void APlayerCharacter::Dash(const FInputActionValue& Value)
{
    // 대시 중이거나 쿨타임 중이면 입력 무시
    if (bIsDashing || bIsDashOnCooldown) return;

    if (DashMontage)
    {
        PlayAnimMontage(DashMontage);
    }

    DashDirection = GetLastMovementInputVector().GetSafeNormal();
    if (DashDirection.IsNearlyZero())
    {
        DashDirection = GetActorForwardVector();
    }

    bIsDashing = true;
    bIsDashOnCooldown = true; // 대시 시작과 동시에 쿨타임 시작

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
        OriginalMaxAcceleration = Movement->MaxAcceleration;

        //  공중 대시를 위해 이동 모드를 Flying으로 변경 (중력 무시)
        Movement->SetMovementMode(MOVE_Flying);

        //  속도 및 가속도 설정
        Movement->MaxFlySpeed = DashSpeed; // Flying 모드일 땐 MaxFlySpeed를 사용.
        Movement->MaxAcceleration = 10000.f;

        //  대시 시작 시 순간적인 속도 초기화 (이전 관성 제거)
        Movement->Velocity = DashDirection * DashSpeed;

        // 대시 종료 타이머
        GetWorldTimerManager().SetTimer(DashTimerHandle, this, &APlayerCharacter::StopDash, DashDuration, false);

        // 쿨타임 타이머 (대시 시작 시점부터 DashCooldown초 후 해제)
        GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, this, &APlayerCharacter::ResetDashCooldown, DashCooldown, false);

        if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
        {
            PlayerController->SkillCooldownTimer_2();
        }
    }
}

void APlayerCharacter::StopDash()
{
    bIsDashing = false;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = OriginalMaxWalkSpeed;
        Movement->MaxAcceleration = OriginalMaxAcceleration;

        Movement->SetMovementMode(MOVE_Walking);

        Movement->Velocity = Movement->Velocity.GetSafeNormal() * OriginalMaxWalkSpeed;
    }
}

// 쿨타임 종료: 다시 대시 가능 상태로 전환
void APlayerCharacter::ResetDashCooldown()
{
    bIsDashOnCooldown = false;
}

void APlayerCharacter::StartFire()
{
    if (CurrentWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartFire Called!"));
        UE_LOG(LogTemp, Warning, TEXT("CurrentWeapon is Valid!"));

        // 실제 발사 로직 호출
        CurrentWeapon->StartFire();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CurrentWeapon is NULL!"));
    }
}

void APlayerCharacter::StartReload()
{
    UE_LOG(LogTemp, Warning, TEXT("[Reload] StartReload Called"));

    if (CurrentWeapon)
    {
        CurrentWeapon->Reload();
    }
}

void APlayerCharacter::StopFire()
{
    if (CurrentWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("StopFire Called!"));
        CurrentWeapon->StopFire();
    }
}

void APlayerCharacter::SetNearbyWeaponPickup(AWeaponPickup* NewPickup)
{
    NearbyWeaponPickup = NewPickup;
}

void APlayerCharacter::Interact()
{
    if (!NearbyWeaponPickup)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Pickup] No NearbyWeaponPickup"));
        return;
    }

    TSubclassOf<AWeaponBase> NewWeaponClass = NearbyWeaponPickup->GetWeaponClass();
    if (!NewWeaponClass)
    {
        return;
    }

    AWeaponPickup* PickupToDestroy = NearbyWeaponPickup;
    NearbyWeaponPickup = nullptr;

    EquipWeapon(NewWeaponClass);

    PickupToDestroy->Destroy();
}

void APlayerCharacter::EquipWeapon(TSubclassOf<AWeaponBase> NewWeaponClass)
{
    if (!NewWeaponClass || !GetWorld())
    {
        return;
    }

    DropCurrentWeapon();

    WeaponClass = NewWeaponClass;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(NewWeaponClass, SpawnParams);
    if (!CurrentWeapon)
    {
        CurrentWeaponType = EWeaponType::None;
        return;
    }
    if (SavedAmmoMap.Contains(NewWeaponClass))
    {
        CurrentWeapon->CurrentAmmo = SavedAmmoMap[NewWeaponClass];
    }

    CurrentWeapon->SetOwner(this);

    // 현재 무기 타입을 ABP에서 읽을 수 있게 저장
    CurrentWeaponType = CurrentWeapon->WeaponType;

    USkeletalMeshComponent* AttachMesh = nullptr;

    // 1인칭 팔 메시가 실제로 들어있으면 팔에 붙이고,
    // 없으면 캐릭터 본체 Mesh의 손 소켓에 붙인다.
    if (FirstPersonArmsMesh && FirstPersonArmsMesh->GetSkeletalMeshAsset())
    {
        AttachMesh = FirstPersonArmsMesh;
    }
    else
    {
        AttachMesh = GetMesh();
    }

    if (AttachMesh)
    {
        CurrentWeapon->AttachToComponent(
            AttachMesh,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            WeaponAttachSocketName
        );
    }
    else
    {
        CurrentWeapon->AttachToComponent(
            CameraComp,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale
        );
    }

    // WeaponData에 저장된 위치/회전/크기 적용
    CurrentWeapon->ApplyWeaponAttachTransform();

    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Ammo();
    }
}

void APlayerCharacter::DropCurrentWeapon()
{
    if (!CurrentWeapon || !GetWorld())
    {
        return;
    }

    SavedAmmoMap.Add(CurrentWeapon->GetClass(), CurrentWeapon->CurrentAmmo);

    if (WeaponPickupClass)
    {
        FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 120.f;
        DropLocation.Z += 30.f;

        FRotator DropRotation = GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AWeaponPickup* DroppedPickup = GetWorld()->SpawnActor<AWeaponPickup>(
            WeaponPickupClass,
            DropLocation,
            DropRotation,
            SpawnParams
        );

        if (DroppedPickup)
        {
            DroppedPickup->SetWeaponClass(CurrentWeapon->GetClass());
        }
    }

    CurrentWeapon->Destroy();
    CurrentWeapon = nullptr;

    // 현재 무기가 없으므로 ABP에는 None 상태 전달
    CurrentWeaponType = EWeaponType::None;
}

void APlayerCharacter::PlayArmsMontage(UAnimMontage* MontageToPlay)
{
    if (!MontageToPlay)
    {
        return;
    }

    USkeletalMeshComponent* MontageMesh = nullptr;

    if (FirstPersonArmsMesh && FirstPersonArmsMesh->GetSkeletalMeshAsset())
    {
        MontageMesh = FirstPersonArmsMesh;
    }
    else
    {
        MontageMesh = GetMesh();
    }

    if (!MontageMesh)
    {
        return;
    }

    if (UAnimInstance* AnimInstance = MontageMesh->GetAnimInstance())
    {
        AnimInstance->Montage_Play(MontageToPlay);
    }
}

void APlayerCharacter::ApplyWeaponRecoil(float RecoilPitch, float RecoilYaw)
{
    // 실제 조준 반동은 컨트롤러 회전에 적용
    AddControllerPitchInput(-RecoilPitch);
    AddControllerYawInput(FMath::RandRange(-RecoilYaw, RecoilYaw));
}

// 스킬 실행 함수 구현
void APlayerCharacter::UseSkillInput()
{
    if (!SkillComp || !SkillComp->CurrentSkill)
    {
        return;
    }

    // 쿨타임 중이면 RPG도 꺼내지 않음
    if (!SkillComp->CurrentSkill->CanUseSkill())
    {
        UE_LOG(LogTemp, Warning, TEXT("[RPG] Skill is on cooldown. RPG will not equip."));
        return;
    }

    // 스킬 사용 중에는 들고 있던 총을 숨기고 RPG 액터를 장착함
    const bool bRPGEquipped = EquipRPG();

    bool bSkillUsed = SkillComp->UseSkill();

    if (bSkillUsed)
    {
        if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
        {
            PlayerController->SkillCooldownTimer();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[RPG] EquipResult: %s / SkillUsed: %s"),
        bRPGEquipped ? TEXT("true") : TEXT("false"),
        bSkillUsed ? TEXT("true") : TEXT("false"));

    // RPG가 장착되었다면 스킬 성공/실패와 관계없이 일정 시간 뒤 원래 총으로 복구
    if (bRPGEquipped)
    {
        GetWorldTimerManager().ClearTimer(RPGUnequipTimerHandle);
        GetWorldTimerManager().SetTimer(
            RPGUnequipTimerHandle,
            this,
            &APlayerCharacter::UnequipRPG,
            RPGEquipDuration,
            false
        );
    }

    // RPG 장착도 실패했고 스킬도 실패했다면 복구만 보장
    if (!bRPGEquipped && !bSkillUsed)
    {
        UnequipRPG();
    }
}

// RPG 액터 장착
bool APlayerCharacter::EquipRPG()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("[RPG] World is NULL"));
        return false;
    }

    if (!RPGWeaponActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RPG] RPGWeaponActorClass is not set!"));
        return false;
    }

    if (!CameraComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RPG] CameraComp is NULL"));
        return false;
    }

    if (EquippedRPGActor)
    {
        EquippedRPGActor->Destroy();
        EquippedRPGActor = nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    EquippedRPGActor = GetWorld()->SpawnActor<AActor>(
        RPGWeaponActorClass,
        CameraComp->GetComponentLocation(),
        CameraComp->GetComponentRotation(),
        SpawnParams
    );

    if (!EquippedRPGActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RPG] Failed to spawn RPG weapon actor!"));
        return false;
    }

    // RPG 액터가 실제로 장착된 뒤에 원래 총을 숨김
    if (CurrentWeapon)
    {
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    // 카메라에 붙인 뒤, BP_PlayerCharacter의 RPGAttach 값으로 위치/회전/크기를 적용
    EquippedRPGActor->AttachToComponent(CameraComp, FAttachmentTransformRules::KeepRelativeTransform);
    EquippedRPGActor->SetActorRelativeLocation(RPGAttachLocation);
    EquippedRPGActor->SetActorRelativeRotation(RPGAttachRotation);
    EquippedRPGActor->SetActorRelativeScale3D(RPGAttachScale);
    EquippedRPGActor->SetActorHiddenInGame(false);

    UE_LOG(LogTemp, Warning, TEXT("[RPG] Equipped RPG Actor: %s"), *EquippedRPGActor->GetName());

    return true;
}

// RPG 장착 해제
void APlayerCharacter::UnequipRPG()
{
    if (EquippedRPGActor)
    {
        EquippedRPGActor->Destroy();
        EquippedRPGActor = nullptr;
    }

    if (CurrentWeapon)
    {
        CurrentWeapon->SetActorHiddenInGame(false);

        // RPG 사용 중 무기 반동 위치가 남아있을 수 있으므로 원래 부착값으로 복구
        CurrentWeapon->ApplyWeaponAttachTransform();
    }

    UE_LOG(LogTemp, Warning, TEXT("[RPG] Unequip RPG"));
}

void APlayerCharacter::ApplyDamage(float DamageAmount)
{
    if (CurrentHealth <= 0.0f)
    {
        return;
    }

    CurrentHealth -= DamageAmount;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
    }

    if (HitEffect)
    {
        const FVector EffectLocation =
            GetActorLocation()
            + GetActorForwardVector() * HitEffectForwardOffset
            + FVector(0.0f, 0.0f, HitEffectZOffset);

        const FRotator EffectRotation = GetActorRotation();

        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            HitEffect,
            EffectLocation,
            EffectRotation
        );
    }

    if (APController* PlayerController = Cast<APController>(GetController()))
    {
        PlayerController->UpdateHUD_HP();

        if (HitCameraShake)
        {
            PlayerController->ClientStartCameraShake(HitCameraShake);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("HP: %f"), CurrentHealth);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.f,
            FColor::Red,
            FString::Printf(TEXT("HP: %f"), CurrentHealth)
        );
    }

    if (CurrentHealth <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player Dead"));
        Destroy();
    }
}
