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

    // 카메라에 무기를 붙였을 때 위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AttachLocation = FVector(20.f, 20.f, -20.f);

    // 카메라에 무기를 붙였을 때 회전
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator AttachRotation = FRotator(10.f, 10.f, 0.f);

    // 카메라에 무기를 붙였을 때 크기
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AttachScale = FVector(0.8f);

    // 발사 시 무기가 순간적으로 밀리는 위치값
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FireRecoilLocationOffset = FVector(-8.f, 0.f, 2.f);

    // 발사 시 무기가 순간적으로 회전하는 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator FireRecoilRotationOffset = FRotator(-3.f, 0.f, 0.f);

    // 발사 후 원래 위치로 돌아오는 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FireRecoilReturnTime = 0.06f;

    // 실제 총 메시의 위치 보정값
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MeshRelativeLocation = FVector::ZeroVector;

    // 실제 총 메시의 회전 보정값
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator MeshRelativeRotation = FRotator::ZeroRotator;

    // 실제 총 메시의 크기 보정값
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MeshRelativeScale = FVector(1.0f);
};
