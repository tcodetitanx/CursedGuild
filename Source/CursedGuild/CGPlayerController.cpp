#include "CGPlayerController.h"
#include "CGGameInstance.h"
#include "CGRootWidget.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"

void ACGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI().SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock));

	UCGGameInstance* GI = Cast<UCGGameInstance>(GetGameInstance());
	if (GI && GEngine && GEngine->GameViewport)
	{
		RootWidget = SNew(SCGRoot).GameInstance(GI);
		GEngine->GameViewport->AddViewportWidgetContent(RootWidget.ToSharedRef(), 10);
	}
}

void ACGPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (RootWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(RootWidget.ToSharedRef());
		RootWidget.Reset();
	}
	Super::EndPlay(EndPlayReason);
}
