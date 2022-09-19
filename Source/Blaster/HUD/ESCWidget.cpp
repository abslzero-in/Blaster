#include "ESCWidget.h"
#include <GameFramework/PlayerController.h>
#include <GameFramework/GameModeBase.h>
#include <Components/Button.h>
#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/Engine.h"



void UESCWidget::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World) {
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController) {
			FInputModeGameAndUI InputModeData;

			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem) {
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UESCWidget::OnDestroySession);
		}
	}

	if (ReturnButton && !ReturnButton->OnClicked.IsBound()) {
		ReturnButton->OnClicked.AddDynamic(this, &UESCWidget::ReturnButtonClicked);
	}
}

bool UESCWidget::Initialize()
{
	if (!Super::Initialize()) {
		return false;
	}

	return true;
}

void UESCWidget::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful) {
		ReturnButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World) {
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode) {
			GameMode->ReturnToMainMenuHost();
		}
		else {
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController) {
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UESCWidget::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (World) {
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController) {
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsBound()) {
		ReturnButton->OnClicked.RemoveDynamic(this, &UESCWidget::ReturnButtonClicked);
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound()) {
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UESCWidget::OnDestroySession);
	}
}

void UESCWidget::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World) {
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController) {
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(PlayerController->GetPawn());
			if (BlasterCharacter) {
				BlasterCharacter->ServerLeaveGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this, &UESCWidget::OnPlayerLeftGame);
			}
			else {
				ReturnButton->SetIsEnabled(true);
			}
		}
	}

}

void UESCWidget::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->DestroySession();
	}
}