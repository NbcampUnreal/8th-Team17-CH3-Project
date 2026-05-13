#include "PlayerCharacter.h"											
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"											
#include "GameFramework/SpringArmComponent.h"											
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

#include "DrawDebugHelpers.h"
#include "TemaProject03/BattelSystem/WeaponBase.h"

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
        CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);
        if (CurrentWeapon)
        {
            CurrentWeapon->SetOwner(this); // 중요: 무기의 Owner를 플레이어로 설정

            // 손에 붙이기 (소켓 이름은 hand_rSocket 기준)
            CurrentWeapon->AttachToComponent(CameraComp, FAttachmentTransformRules::SnapToTargetIncludingScale);

            CurrentWeapon->SetActorRelativeLocation(FVector(20.f, 20.f, -20.f));

            CurrentWeapon->SetActorRelativeRotation(FRotator(10.f, 10.f, 0.f));

            // 크기 조정
            CurrentWeapon->SetActorScale3D(FVector(0.8f));
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
    }
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

        // 발사 애니메이션 재생 (없어도 발사는 되도록 if문으로 분리)
        if (FireMontage)
        {
            PlayAnimMontage(FireMontage);
        }

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
    if (CurrentWeapon)
    {
        // 리로드 애니메이션 재생
        if (ReloadMontage)
        {
            PlayAnimMontage(ReloadMontage);
        }
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

void APlayerCharacter::ApplyDamage(float DamageAmount)
{
    CurrentHealth -= DamageAmount;


    UE_LOG(LogTemp, Warning, TEXT("HP: %f"), CurrentHealth);

    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("HP: %f"), CurrentHealth));

    if (CurrentHealth <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy Dead"));

        Destroy();
    }
}
