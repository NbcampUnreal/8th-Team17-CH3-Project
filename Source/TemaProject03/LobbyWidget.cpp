#include "LobbyWidget.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    CurrentMenuState = ELobbyMenuState::MainLobby;
    SelectedCharacterIndex = INDEX_NONE;
    SelectedTraitIndex = INDEX_NONE;

    ShowMainLobby();
    SetCharacterNextButtonEnabled(false);
    SetFinalStartButtonEnabled(false);
}

void ULobbyWidget::OnClickGameStart()
{
    CurrentMenuState = ELobbyMenuState::CharacterSelect;
    ShowCharacterSelect();
}

void ULobbyWidget::OnClickGameExit()
{
    UKismetSystemLibrary::QuitGame(
        this,
        GetOwningPlayer(),
        EQuitPreference::Quit,
        false
    );
}

void ULobbyWidget::OnSelectCharacter(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= CharacterSlotCount)
    {
        return;
    }

    SelectedCharacterIndex = SlotIndex;
    SetCharacterNextButtonEnabled(true);
}

void ULobbyWidget::OnClickCharacterNext()
{
    if (SelectedCharacterIndex == INDEX_NONE)
    {
        return;
    }

    CurrentMenuState = ELobbyMenuState::TraitSelect;
    ShowTraitSelect();
    SetFinalStartButtonEnabled(SelectedTraitIndex != INDEX_NONE);
}

void ULobbyWidget::OnSelectTrait(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= TraitSlotCount)
    {
        return;
    }

    SelectedTraitIndex = SlotIndex;
    SetFinalStartButtonEnabled(true);
}

void ULobbyWidget::OnClickFinalGameStart()
{
    if (SelectedCharacterIndex == INDEX_NONE || SelectedTraitIndex == INDEX_NONE)
    {
        return;
    }

    UGameplayStatics::OpenLevel(this, GameLevelName);
}

void ULobbyWidget::OnClickBack()
{
    if (CurrentMenuState == ELobbyMenuState::TraitSelect)
    {
        CurrentMenuState = ELobbyMenuState::CharacterSelect;
        ShowCharacterSelect();
        return;
    }

    if (CurrentMenuState == ELobbyMenuState::CharacterSelect)
    {
        CurrentMenuState = ELobbyMenuState::MainLobby;
        ShowMainLobby();
    }
}
