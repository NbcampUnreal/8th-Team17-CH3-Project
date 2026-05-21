#include "RoomManager.h"
#include "Portal.h"

ARoomManager::ARoomManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARoomManager::OpenLinkedPortals()
{
    for (APortal* Portal : LinkedPortals)
    {
        if (Portal)
        {
            Portal->SetPortalActive(true);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Room Cleared! Portals Opened."));
}
