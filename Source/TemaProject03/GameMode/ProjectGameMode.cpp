#include "ProjectGameMode.h"
#include "TemaProject03/Player/PlayerCharacter.h"
#include "TemaProject03/Player/PController.h"

AProjectGameMode::AProjectGameMode()
{

    DefaultPawnClass = APlayerCharacter::StaticClass();

    PlayerControllerClass = APController::StaticClass();
}
