#include "PController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "PlayerCharacter.h"

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
            UpdateHUD();
        }
    }
}

void APController::UpdateHUD()
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
