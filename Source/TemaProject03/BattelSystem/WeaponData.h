#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponData.generated.h"

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
    UStaticMesh* WeaponMesh;

};
