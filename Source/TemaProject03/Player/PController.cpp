#include "PController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"
#include "TemaProject03/BattelSystem/WeaponBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "SkillComponent.h"

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
        if (UProgressBar* HealthBar = Cast<UProgressBar>(HUDWidgetInstance->GetWidgetFromName(TEXT("HealthBar"))))
        {
            if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
            {
                float HealthRatio = PlayerCharacter->GetCurrentHealth() / PlayerCharacter->GetMaxHealth();
                HealthBar->SetPercent(HealthRatio);
            }
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

void APController::SkillCooldownTimer()
{
    FTimerDelegate TimerDel;
    TimerDel.BindUObject(this, &APController::UpdateHUD_SkillCooldown, true);
    GetWorldTimerManager().SetTimer(SkillCooldownTimerHandle, TimerDel, 0.1f, true);
}

void APController::UpdateHUD_SkillCooldown(bool bIsOnCooldown)
{
    if (!HUDWidgetInstance) return;

    if (bIsOnCooldown)
    {
        if (UTextBlock* SkillCoolTimeText = Cast<UTextBlock>(HUDWidgetInstance->GetWidgetFromName(TEXT("SkillCoolTimeText"))))
        {
            if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
            {
                if (USkillComponent* SkillComp = PlayerCharacter->GetSkillComponent())
                {
                    UFunction* ShowCoolTimeText = HUDWidgetInstance->FindFunction(FName("ShowCoolTimeText"));
                    if (ShowCoolTimeText)
                    {
                        HUDWidgetInstance->ProcessEvent(ShowCoolTimeText, nullptr);
                    }

                    float RemainingTime = SkillComp->GetRemainingCooldown();                    

                    if (RemainingTime <= 0.0f)
                    {
                        GetWorldTimerManager().ClearTimer(SkillCooldownTimerHandle); // 알람 끄기
                        UpdateHUD_SkillCooldown(false);                              // false로 보내서 UI 숨기기
                        return;                                                      // 함수 종료
                    }

                    SkillCoolTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), RemainingTime)));

                    if (UProgressBar* SkillCoolTimeBar = Cast<UProgressBar>(HUDWidgetInstance->GetWidgetFromName(TEXT("SkillCoolTimeBar"))))
                    {                     
                        float CooldownRatio = 1.0f - (RemainingTime / PlayerCharacter->GetOriginalSkillCooldown());
                        SkillCoolTimeBar->SetPercent(CooldownRatio);
                    }
                }
            }
        }
    }
    else
    {
        UFunction* HideCoolTimeText = HUDWidgetInstance->FindFunction(FName("HideCoolTimeText"));
        if (HideCoolTimeText)
        {
            HUDWidgetInstance->ProcessEvent(HideCoolTimeText, nullptr);
        }
    }

}

void APController::SkillCooldownTimer_2()
{
    FTimerDelegate TimerDel;
    TimerDel.BindUObject(this, &APController::UpdateHUD_SkillCooldown_2, true);
    GetWorldTimerManager().SetTimer(SkillCooldownTimerHandle_2, TimerDel, 0.1f, true);
}

void APController::UpdateHUD_SkillCooldown_2(bool bIsOnCooldown)
{
    if (!HUDWidgetInstance) return;

    if (bIsOnCooldown)
    {
        if (UTextBlock* SkillCoolTimeText = Cast<UTextBlock>(HUDWidgetInstance->GetWidgetFromName(TEXT("SkillCoolTimeText_2"))))
        {
            if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
            {
                UFunction* ShowCoolTimeText = HUDWidgetInstance->FindFunction(FName("ShowCoolTimeText_2"));
                if (ShowCoolTimeText)
                {
                    HUDWidgetInstance->ProcessEvent(ShowCoolTimeText, nullptr);
                }

                float RemainingTime = PlayerCharacter->GetRemainingCooldown();

                if (RemainingTime <= 0.0f)
                {
                    GetWorldTimerManager().ClearTimer(SkillCooldownTimerHandle_2); // 알람 끄기
                    UpdateHUD_SkillCooldown_2(false);                              // false로 보내서 UI 숨기기
                    return;                                                        // 함수 종료
                }

                SkillCoolTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), RemainingTime)));

                if (UProgressBar* SkillCoolTimeBar = Cast<UProgressBar>(HUDWidgetInstance->GetWidgetFromName(TEXT("SkillCoolTimeBar_2"))))
                {
                    float CooldownRatio = 1.0f - (RemainingTime / PlayerCharacter->GetDashCooldown());
                    SkillCoolTimeBar->SetPercent(CooldownRatio);
                }
            }
        }
    }
    else
    {
        UFunction* HideCoolTimeText = HUDWidgetInstance->FindFunction(FName("HideCoolTimeText_2"));
        if (HideCoolTimeText)
        {
            HUDWidgetInstance->ProcessEvent(HideCoolTimeText, nullptr);
        }
    }

}

void APController::ShowUpgradeChoice()
{
    TriggerUICustomEvent(FName("ShowUpgradeChoice"));

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    SetInputMode(InputMode);

    SetPause(true);
}

void APController::SelectHealthUpgrade()
{
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->UpgradeMaxHealth(UpgradeAmount);
    }

    TriggerUICustomEvent(FName("HideUpgradeChoice"));

    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());

    SetPause(false);
}

void APController::SelectAttackUpgrade()
{
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->UpgradeAttack(UpgradeAmount);
    }

    TriggerUICustomEvent(FName("HideUpgradeChoice"));

    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());

    SetPause(false);
}
