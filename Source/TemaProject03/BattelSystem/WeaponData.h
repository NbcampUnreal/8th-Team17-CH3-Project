// WeponData.h

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

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MagazineSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReloadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FireRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFireType FireType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* ProjectileMesh;
};
