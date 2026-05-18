#include "PController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"
#include "TemaProject03/BattelSystem/WeaponBase.h"

APController::APController()
    : HUDWidgetClass(nullptr),
    HUDWidgetInstance(nullptr)
{

}

void APController::BeginPlay()
{
    Super::BeginPlay();

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    if (HUDWidgetClass)
    {
        HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
        if (HUDWidgetInstance)
        {
            HUDWidgetInstance->AddToViewport();
            UpdateHUD_HP();
        }
    }
}

void APController::UpdateHUD_HP()
{
    if (HUDWidgetInstance)
    {
        if(UProgressBar* HealthBar = Cast<UProgressBar>(HUDWidgetInstance->GetWidgetFromName(TEXT("HealthBar"))))
            if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
            {
                float HealthRatio = PlayerCharacter->GetCurrentHealth() / PlayerCharacter->GetMaxHealth();
                HealthBar->SetPercent(HealthRatio);
            }
    }
}

void APController::UpdateHUD_Ammo()
{
    if (HUDWidgetInstance)
    {
        if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
        {
            if (AWeaponBase* Weapon = PlayerCharacter->CurrentWeapon)
            {                 
                if (UTextBlock* MaxAmmoText = Cast<UTextBlock>(HUDWidgetInstance->GetWidgetFromName(TEXT("MaxAmmo"))))
                {
                    MaxAmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Weapon->GetMaxAmmo())));
                }
                if (UTextBlock* CurrentAmmoText = Cast<UTextBlock>(HUDWidgetInstance->GetWidgetFromName(TEXT("CurrentAmmo"))))
                {
                    CurrentAmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Weapon->GetCurrentAmmo())));
                }
            }
        }       
    }
}


void APController::TriggerUICustomEvent(FName EventName)
{
    if (HUDWidgetInstance)
    {
        UFunction* CustomEvent = HUDWidgetInstance->FindFunction(EventName);

        if (CustomEvent)
        {
            HUDWidgetInstance->ProcessEvent(CustomEvent, nullptr);
            UE_LOG(LogTemp, Warning, TEXT("TriggerUICustomEvent Call is Succeeded."))
        }
    }
}

void APController::UpdateHUD_Reload(bool bIsReload)
{
    if (!HUDWidgetInstance) return;

    if (bIsReload)
    {
        UFunction* PlayReloadAnim = HUDWidgetInstance->FindFunction(FName("PlayReloadAnim"));        
        if (PlayReloadAnim)
        {
            HUDWidgetInstance->ProcessEvent(PlayReloadAnim, nullptr);
        }        
    }
    else
    {
        UFunction* EndReloadAnim = HUDWidgetInstance->FindFunction(FName("EndReloadAnim"));
        if (EndReloadAnim)
        {
            HUDWidgetInstance->ProcessEvent(EndReloadAnim, nullptr);
        }        
    }
}
