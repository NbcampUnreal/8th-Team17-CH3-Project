#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponData.generated.h"

UENUM(BlueprintType)
enum class EFireType : uint8
{
    Single UMETA(DisplayName = "Single"),
    Auto UMETA(DisplayName = "Auto"),
    Shotgun UMETA(DisplayName = "Shotgun"),
    Bow UMETA(DisplayName = "Bow"),
    Bazooka UMETA(DisplayName = "Bazooka")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None UMETA(DisplayName = "None"),
    Pistol UMETA(DisplayName = "Pistol"),
    Rifle UMETA(DisplayName = "Rifle"),
    Shotgun UMETA(DisplayName = "Shotgun"),
    Sniper UMETA(DisplayName = "Sniper")
};

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MagazineSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReloadTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FireRate = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFireType FireType = EFireType::Single;

    // 카메라 위쪽 반동
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoilPitch = 1.5f;

    // 카메라 좌우 반동
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoilYaw = 0.3f;
};
