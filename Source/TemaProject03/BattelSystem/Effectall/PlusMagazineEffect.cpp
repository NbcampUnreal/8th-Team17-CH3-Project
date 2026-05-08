#include "PlusMagazineEffect.h"

int32 UPlusMagazineEffect::ModifyMagazineSize(int32 MagazineSize)
{
    int32 NewSize = FMath::RoundToInt(MagazineSize * 1.3f);

    UE_LOG(LogTemp, Warning, TEXT("Magazine Increased: %d"), NewSize);

    return NewSize;
}
