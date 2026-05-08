#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

UENUM(BlueprintType)
enum class ELobbyMenuState : uint8
{
    MainLobby,
    CharacterSelect,
    TraitSelect
};

UCLASS()
class TEMAPROJECT03_API ULobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable)
    void OnClickGameStart();

    UFUNCTION(BlueprintCallable)
    void OnClickGameExit();

    UFUNCTION(BlueprintCallable)
    void OnSelectCharacter(int32 SlotIndex);

    UFUNCTION(BlueprintCallable)
    void OnClickCharacterNext();

    UFUNCTION(BlueprintCallable)
    void OnSelectTrait(int32 SlotIndex);

    UFUNCTION(BlueprintCallable)
    void OnClickFinalGameStart();

    UFUNCTION(BlueprintCallable)
    void OnClickBack();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    int32 CharacterSlotCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    int32 TraitSlotCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    FName GameLevelName = "GameLevel";

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    int32 SelectedCharacterIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    int32 SelectedTraitIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    ELobbyMenuState CurrentMenuState = ELobbyMenuState::MainLobby;

    UFUNCTION(BlueprintImplementableEvent)
    void ShowMainLobby();

    UFUNCTION(BlueprintImplementableEvent)
    void ShowCharacterSelect();

    UFUNCTION(BlueprintImplementableEvent)
    void ShowTraitSelect();

    UFUNCTION(BlueprintImplementableEvent)
    void SetCharacterNextButtonEnabled(bool bEnabled);

    UFUNCTION(BlueprintImplementableEvent)
    void SetFinalStartButtonEnabled(bool bEnabled);
};
