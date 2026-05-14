#include "Skill_Solider.h"
#include "RPGProjectile.h"
#include "TemaProject03/Player/PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"

USkill_Solider::USkill_Solider()
{
    // 스킬 기본 정보 설정
    SkillName = TEXT("RPG");
    Cooldown = 10.0f;
}

// 플레이어의 현재 공격력에 비례하여 스킬의 실제 데미지를 계산
float USkill_Solider::CalculateDamage(AActor* Owner)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(Owner);
    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Soldier Skill] Owner is not PlayerCharacter! Using default damage."));
        return 100.0f; // 플레이어가 아닐 경우 기본 데미지 반환
    }

    // 플레이어 공격력의 67%를 스킬 데미지로 설정 (계수 조정 가능)
    float Damage = Player->CharacterAttack * 0.67f;
    UE_LOG(LogTemp, Warning, TEXT("[Soldier Skill] CharacterAttack: %.1f, SkillDamage: %.1f"),
        Player->CharacterAttack, Damage);

    return Damage;
}

// 스킬 실행 메인 로직 (데미지 계산 -> 탄환 스폰)
bool USkill_Solider::ActivateSkill(AActor* Owner)
{
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Soldier Skill] Owner is NULL!"));
        return false;
    }

    if (!RocketClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[Soldier Skill] RocketClass is NOT assigned in Blueprint!"));
        return false;
    }

    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Soldier Skill] World is NULL!"));
        return false;
    }

    // 공격력 계산
    SkillDamage = CalculateDamage(Owner);

    UE_LOG(LogTemp, Warning, TEXT("[Soldier Skill] RPG Fired! Damage: %.1f"), SkillDamage);

    ACharacter* Character = Cast<ACharacter>(Owner);
    UCameraComponent* Camera = Character ? Character->FindComponentByClass<UCameraComponent>() : nullptr;

    FVector SpawnLocation;

    // 장착된 RPG 액터에 Muzzle 컴포넌트가 있으면 그 위치에서 탄두를 스폰
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(Owner))
    {
        FTransform MuzzleTransform;
        if (Player->GetRPGMuzzleTransform(MuzzleTransform))
        {
            SpawnLocation = MuzzleTransform.GetLocation();
        }
        else
        {
            SpawnLocation = Camera
                ? Camera->GetComponentLocation() + (Camera->GetForwardVector() * 100.f)
                : Owner->GetActorLocation() + (Owner->GetActorForwardVector() * 100.f);
        }
    }
    else
    {
        SpawnLocation = Camera
            ? Camera->GetComponentLocation() + (Camera->GetForwardVector() * 100.f)
            : Owner->GetActorLocation() + (Owner->GetActorForwardVector() * 100.f);
    }

    // 발사 방향은 카메라 에임 방향을 사용
    FRotator SpawnRotation = Camera
        ? Camera->GetComponentRotation()
        : Owner->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Cast<APawn>(Owner);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 탄환 액터 생성
    ARPGProjectile* Projectile = World->SpawnActor<ARPGProjectile>(
        RocketClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (!Projectile)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Soldier Skill] Rocket Projectile Spawn Failed!"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Soldier Skill] Rocket Projectile Spawned Success!"));

    // 탄두가 폭발 정보를 직접 들고 있도록 전달
    Projectile->InitProjectile(SkillDamage, ExplosionRadius, HitCount);
    Projectile->SetFireRange(FireRange, RocketSpeed);
    Projectile->SetProjectileSpeed(RocketSpeed);

    return true;
}
