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

#include "TemaProject03/BattelSystem/WeaponBox.h"
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
    if (PistolClass)
    {
        PistolWeapon =
            GetWorld()->SpawnActor<AWeaponBase>(PistolClass);

        if (PistolWeapon)
        {
            PistolWeapon->SetOwner(this);

            EquipWeapon(PistolWeapon);
        }
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

        if (SelectWeapon1Action)
        {
            EnhancedInput->BindAction(
                SelectWeapon1Action,
                ETriggerEvent::Started,
                this,
                &APlayerCharacter::SelectRandomWeapon1
            );
        }

        if (SelectWeapon2Action)
        {
            EnhancedInput->BindAction(
                SelectWeapon2Action,
                ETriggerEvent::Started,
                this,
                &APlayerCharacter::SelectRandomWeapon2
            );
        }

        if (SelectWeapon3Action)
        {
            EnhancedInput->BindAction(
                SelectWeapon3Action,
                ETriggerEvent::Started,
                this,
                &APlayerCharacter::SelectRandomWeapon3
            );
        }

        if (SetSlot1Action)
        {
            EnhancedInput->BindAction(
                SetSlot1Action,
                ETriggerEvent::Started,
                this,
                &APlayerCharacter::PutWeaponInSlot1
            );
        }

        if (SetSlot2Action)
        {
            EnhancedInput->BindAction(
                SetSlot2Action,
                ETriggerEvent::Started,
                this,
                &APlayerCharacter::PutWeaponInSlot2
            );
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
    if (WeaponClass)
    {
        AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);

        EquipWeapon(NewWeapon);
    }
}

void APlayerCharacter::EquipRifle()
{
    EquipWeapon(Slot1Weapon);
}

void APlayerCharacter::EquipShotgun()
{
    EquipWeapon(Slot2Weapon);
}

void APlayerCharacter::EquipPistol()
{
    EquipWeapon(PistolWeapon);
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
    // RPG 사용 중에는 일반 총 발사를 막음
    if (EquippedRPGActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RPG] Cannot fire normal weapon while RPG is equipped."));
        return;
    }

    if (CurrentWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartFire Called!"));
        UE_LOG(LogTemp, Warning, TEXT("CurrentWeapon is Valid!"));

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
    UE_LOG(LogTemp, Warning, TEXT("INTERACT KEY PRESSED"));

    if (NearbyWeaponBox)
    {
        NearbyWeaponBox->OpenWeaponBox(this);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Interact] No nearby weapon box."));
}

void APlayerCharacter::EquipWeapon(AWeaponBase* NewWeapon)
{
    if (!NewWeapon || !GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipWeapon] NewWeapon is NULL"));
        return;
    }

    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
        CurrentWeapon->SetActorHiddenInGame(true);
        CurrentWeapon->SetActorEnableCollision(false);
    }

    CurrentWeapon = NewWeapon;
    CurrentWeapon->SetOwner(this);
    CurrentWeapon->SetActorHiddenInGame(false);
    CurrentWeapon->SetActorEnableCollision(false);

    CurrentWeaponType = CurrentWeapon->WeaponType;

    USkeletalMeshComponent* AttachMesh = nullptr;

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

    CurrentWeapon->ApplyWeaponAttachTransform();
    CurrentWeapon->InitWeapon(this);

    if (APController* PlayerController = Cast<APController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->UpdateHUD_Ammo();
    }

    UE_LOG(LogTemp, Warning, TEXT("[EquipWeapon] Equipped: %s"), *CurrentWeapon->GetName());
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

void APlayerCharacter::SetNearbyWeaponBox(AWeaponBox* NewBox)
{
    NearbyWeaponBox = NewBox;
}

void APlayerCharacter::SetWeaponToSlot(AWeaponBase* NewWeapon, int32 SlotIndex)
{
    if (!NewWeapon)
    {
        return;
    }

    switch (SlotIndex)
    {
    case 1:
        Slot1Weapon = NewWeapon;
        break;

    case 2:
        Slot2Weapon = NewWeapon;
        break;
    }

    EquipWeapon(NewWeapon);
}

void APlayerCharacter::SelectWeapon(AWeaponBase* NewWeapon)
{
    if (!NewWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] Selected weapon is NULL"));
        return;
    }

    PendingSelectedWeapon = NewWeapon;

    UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] Weapon Selected: %s"), *NewWeapon->GetName());
}

void APlayerCharacter::SetSelectedWeapon(TSubclassOf<AWeaponBase> NewWeapon)
{
    SelectedWeapon = NewWeapon;
}

void APlayerCharacter::SelectRandomWeapon1()
{
    if (CurrentRandomWeapons.Num() > 0)
    {
        SelectWeapon(CurrentRandomWeapons[0]);

        UE_LOG(LogTemp, Warning, TEXT("Selected Random Weapon 1"));
    }
}

void APlayerCharacter::SelectRandomWeapon2()
{
    if (CurrentRandomWeapons.Num() > 1)
    {
        SelectWeapon(CurrentRandomWeapons[1]);

        UE_LOG(LogTemp, Warning, TEXT("Selected Random Weapon 2"));
    }
}

void APlayerCharacter::SelectRandomWeapon3()
{
    if (CurrentRandomWeapons.Num() > 2)
    {
        SelectWeapon(CurrentRandomWeapons[2]);

        UE_LOG(LogTemp, Warning, TEXT("Selected Random Weapon 3"));
    }
}

void APlayerCharacter::PutWeaponInSlot1()
{
    if (!PendingSelectedWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] No pending weapon for Slot 1"));
        return;
    }

    SetWeaponToSlot(PendingSelectedWeapon, 1);
    PendingSelectedWeapon = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] Weapon Put In Slot 1"));
}

void APlayerCharacter::PutWeaponInSlot2()
{
    if (!PendingSelectedWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] No pending weapon for Slot 2"));
        return;
    }

    SetWeaponToSlot(PendingSelectedWeapon, 2);
    PendingSelectedWeapon = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("[WeaponBox] Weapon Put In Slot 2"));
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

    // RPG 사용 중에는 일반 총 발사를 멈추고 총을 숨김
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    // 지금 실제로 쓰는 팔/몸 Mesh는 FirstPersonArmsMesh가 아니라 기본 Mesh라서 GetMesh()를 숨김
    if (GetMesh())
    {
        GetMesh()->SetHiddenInGame(true, true);
        GetMesh()->SetVisibility(false, true);
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

    // RPG 사용이 끝나면 기본 Mesh를 다시 보이게 복구
    if (GetMesh())
    {
        GetMesh()->SetHiddenInGame(false, true);
        GetMesh()->SetVisibility(true, true);
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
        Die();
    }
}

void APlayerCharacter::RestoreHealthByPercent(float Percent)
{
    if (CurrentHealth <= 0.0f || MaxHealth <= 0.0f)
    {
        return;
    }

    const float HealAmount = MaxHealth * Percent;
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);

    if (APController* PlayerController = Cast<APController>(GetController()))
    {
        PlayerController->UpdateHUD_HP();
    }

    UE_LOG(LogTemp, Warning, TEXT("[Item] Heal %.1f%% / HP: %.1f / %.1f"), Percent * 100.0f, CurrentHealth, MaxHealth);
}

void APlayerCharacter::UpgradeMaxHealth(float Amount)
{
    MaxHealth += Amount;
    CurrentHealth += Amount;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    if (APController* PlayerController = Cast<APController>(GetController()))
    {
        PlayerController->UpdateHUD_HP();
    }

    UE_LOG(LogTemp, Warning, TEXT("[Upgrade] MaxHealth increased by %.1f / MaxHealth: %.1f"), Amount, MaxHealth);
}

void APlayerCharacter::UpgradeAttack(float Amount)
{
    CharacterAttack += Amount;

    UE_LOG(LogTemp, Warning, TEXT("[Upgrade] CharacterAttack increased by %.1f / Attack: %.1f"), Amount, CharacterAttack);
}

void APlayerCharacter::Die()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;

    UE_LOG(LogTemp, Warning, TEXT("Player Dead"));

    StopFire();

    if (CurrentWeapon)
    {
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    if (EquippedRPGActor)
    {
        EquippedRPGActor->SetActorHiddenInGame(true);
    }

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        DisableInput(PlayerController);

        // 죽을 때 1인칭 카메라를 3인칭 위치로 뒤로 빼서 사망 애니메이션이 보이게
        if (SpringArmComp)
        {
            SpringArmComp->TargetArmLength = 350.0f;
            SpringArmComp->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
            SpringArmComp->bUsePawnControlRotation = false;
            SpringArmComp->bDoCollisionTest = false;
        }

        if (CameraComp)
        {
            CameraComp->bUsePawnControlRotation = false;
            CameraComp->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));
        }
    }

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }

    if (USkeletalMeshComponent* DeathMesh = GetMesh())
    {
        DeathMesh->SetHiddenInGame(false);
        DeathMesh->SetOwnerNoSee(false);
    }

    // 죽는 애니메이션을 잠깐 보여준 뒤 화면을 검게 페이드아웃
    GetWorldTimerManager().SetTimer(
        DeathFadeTimerHandle,
        this,
        &APlayerCharacter::StartDeathFade,
        DeathFadeDelay,
        false
    );
}

void APlayerCharacter::StartDeathFade()
{
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        PlayerController->ClientSetCameraFade(
            true,
            FColor::Black,
            FVector2D(0.0f, 1.0f),
            DeathFadeDuration,
            false,
            true
        );
    }
}
