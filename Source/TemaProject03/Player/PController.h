#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PController.generated.h"

UCLASS()
class TEMAPROJECT03_API APController : public APlayerController
{
    GENERATED_BODY()

public:
    APController();

    // 에디터에서 만든 IMC 에셋을 넣을 변수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<UUserWidget> HUDWidgetClass;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
    UUserWidget* HUDWidgetInstance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<UUserWidget> LobbyWidgetClass;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
    UUserWidget* LobbyWidgetInstance;

    FTimerHandle SkillCooldownTimerHandle;
    FTimerHandle SkillCooldownTimerHandle_2;
    FTimerHandle ReloadTimerHandle;

    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowGameHUD();
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void ShowLobbyHUD();
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void StartGame();

    UFUNCTION(BlueprintCallable)
    void UpdateHUD_HP();

    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void ShowUpgradeChoice();

    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void SelectHealthUpgrade();

    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void SelectAttackUpgrade();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
    float UpgradeAmount = 10.0f;

    void UpdateHUD_Ammo();
    void StartReloadUI();
    void UpdateHUD_Reload();
    void SkillCooldownTimer();
    void SkillCooldownTimer_2();
    void UpdateHUD_SkillCooldown(bool bIsOnCooldown);
    void UpdateHUD_SkillCooldown_2(bool bIsOnCooldown);
    void TriggerUICustomEvent(FName EventName);

protected:
    virtual void BeginPlay() override;
};
