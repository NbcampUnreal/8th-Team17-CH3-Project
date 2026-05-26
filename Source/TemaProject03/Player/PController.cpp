#include "PController.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"
#include "TemaProject03/BattelSystem/WeaponBase.h"
#include "SkillComponent.h"

APController::APController()
    : HUDWidgetClass(nullptr),
    HUDWidgetInstance(nullptr),
    LobbyWidgetClass(nullptr),
    LobbyWidgetInstance(nullptr)
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

    for (TActorIterator<ACameraActor> Camera(GetWorld()); Camera; ++Camera)
    {
        ACameraActor* LobbyCamera = *Camera;

        if (LobbyCamera->ActorHasTag(FName("LobbyCamera")))
        {
            SetViewTargetWithBlend(LobbyCamera);
            break;
        }
    }

    if (HUDWidgetClass)
    {
        HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
    }

    ShowLobbyHUD();    
}

void APController::ShowGameHUD()
{
    if (!HUDWidgetInstance)
    {
        if (HUDWidgetClass)
        {
            HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
        }        
    }

    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->AddToViewport();

        bShowMouseCursor = false;
        SetInputMode(FInputModeGameOnly());

        UpdateHUD_HP();
    }   
}

void APController::ShowLobbyHUD()
{
    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->RemoveFromParent();
        HUDWidgetInstance = nullptr;
    }

    if (LobbyWidgetInstance)
    {
        LobbyWidgetInstance->RemoveFromParent();
        LobbyWidgetInstance = nullptr;
    }

    if (LobbyWidgetClass)
    {
        LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
        if (LobbyWidgetInstance)
        {
            LobbyWidgetInstance->AddToViewport();

            bShowMouseCursor = true;
            SetInputMode(FInputModeUIOnly());
        }
    }
}

void APController::StartGame()
{
    if (LobbyWidgetInstance)
    {
        LobbyWidgetInstance->RemoveFromParent();
    }

    if (APawn* MyPawn = GetPawn())
    {
        SetViewTargetWithBlend(MyPawn, 0.0f);
    }

    ShowGameHUD();
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

void APController::StartReloadUI()
{
    if (!HUDWidgetInstance) return;

    // 1. 장전 애니메이션 1번 실행
    UFunction* PlayReload = HUDWidgetInstance->FindFunction(FName("PlayReload"));
    if (PlayReload)
    {
        HUDWidgetInstance->ProcessEvent(PlayReload, nullptr);
    }

    // 2. 0.1초마다 UpdateHUD_Reload를 반복 실행하도록 타이머 켬
    GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &APController::UpdateHUD_Reload, 0.1f, true);
}

void APController::UpdateHUD_Reload()
{
    if (!HUDWidgetInstance) return;

    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
    {
        if (PlayerCharacter->CurrentWeapon)
        {
            float RemainingTime = PlayerCharacter->CurrentWeapon->GetRemainingReloadTime();

            // 1. 장전이 완료되었다면 (남은 시간이 0 이하라면)
            if (RemainingTime <= 0.0f)
            {
                GetWorldTimerManager().ClearTimer(ReloadTimerHandle); // 알람 끄기

                // 종료 애니메이션 1번 실행 후 숨기기
                UFunction* EndReload = HUDWidgetInstance->FindFunction(FName("EndReload"));
                if (EndReload)
                {
                    HUDWidgetInstance->ProcessEvent(EndReload, nullptr);
                }
                return; // 함수 종료
            }

            // 2. 장전 중이라면 프로그레스 바 실시간 갱신
            if (UProgressBar* ReloadBar = Cast<UProgressBar>(HUDWidgetInstance->GetWidgetFromName(TEXT("ReloadBar"))))
            {
                float CooldownRatio = 1.0f - (RemainingTime / PlayerCharacter->CurrentWeapon->WeaponData.ReloadTime);
                ReloadBar->SetPercent(CooldownRatio);
            }
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
