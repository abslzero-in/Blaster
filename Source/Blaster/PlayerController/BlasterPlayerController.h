// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDBountyPlayer();
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(int32 Ammo);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleGameOver();
	void SetHUDBounty();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void SetHUDTime();

	void PollInit();

	/**
	* Sync time between client and server
	*/

	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float GameOver, float StartingTime);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);
	void ShowESCMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	// esc menu

	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ESCMenuWidget;

	UPROPERTY()
	class UESCWidget* ESCMenu;

	bool bESCMenuOpen = false;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float GameOverTime = 0.f;
	uint32 CountdownInt = 0;
	
	UPROPERTY(Replicated)
	class ABlasterPlayerState* CurrentBountyPlayerState;
	
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	UPROPERTY()
	class ABlasterGameState* BlasterGameState;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeCarriedAmmo = false;
	bool bInitializeWeaponAmmo = false;

	float HUDCarriedAmmo;
	float HUDWeaponAmmo;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;

	float HUDShield;
	float HUDMaxShield;

	UPROPERTY(Replicated)
	bool bBountyChanged = false;

	float HighPingRunningTime = 0.f;
	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere, Category = Ping)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere, Category = Ping)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere, Category = Ping)
	float HighPingThreshold = 100.f;
};
