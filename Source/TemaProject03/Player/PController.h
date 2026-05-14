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

    void UpdateHUD();
    void TriggerUICustomEvent(FName EventName);

protected:
    virtual void BeginPlay() override;
};
