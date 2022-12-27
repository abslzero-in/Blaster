// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blaster/Weapon/WeaponTypes.h"


#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KillCount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathCount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoCount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoCount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TopPlayerName;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetHUDWeaponImage(EWeaponType WeaponType, int32 WeaponSlot);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SwapHUDWeaponImage();
};
