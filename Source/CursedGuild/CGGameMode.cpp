#include "CGGameMode.h"
#include "CGPlayerController.h"
#include "GameFramework/SpectatorPawn.h"

ACGGameMode::ACGGameMode()
{
	PlayerControllerClass = ACGPlayerController::StaticClass();
	DefaultPawnClass = ASpectatorPawn::StaticClass();
}
