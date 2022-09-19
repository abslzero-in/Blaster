#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ESCWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UESCWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void MenuSetup();
	void MenuTearDown();


protected:

	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();

private:

	UPROPERTY(Meta = (BindWidget))
	class UButton* ReturnButton;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY()
	class APlayerController* PlayerController;

};
