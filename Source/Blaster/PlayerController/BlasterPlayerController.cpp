// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/Announcement.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include <Net/UnrealNetwork.h>
#include <Kismet/GameplayStatics.h>
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameMode/LobbyGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Engine/Engine.h"
#include <Components/Image.h>
#include "Blaster/HUD/ESCWidget.h"
#include "Blaster/Weapon/WeaponTypes.h"



void ABlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				BlasterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				BlasterHUD->AddElimAnnouncement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("ESCMenu", IE_Pressed, this, &ABlasterPlayerController::ShowESCMenu);

}


void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	SetHUDBountyPlayer();
	PollInit();
	CheckTimeSync(DeltaTime);
	CheckPing(DeltaTime);
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, CurrentBountyPlayerState); 
	DOREPLIFETIME(ABlasterPlayerController, bBountyChanged);
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1);
		BlasterHUD->CharacterOverlay->PlayAnimation(
			BlasterHUD->CharacterOverlay->HighPingAnimation, 
			0.f,
			5);
	}
}

void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0);
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation)) {
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency) {
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState) {
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold) {		// ping is compressed, mul by 4 to get actual value
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else {
				ServerReportPingStatus(false);	
			}
		}
		HighPingRunningTime = 0.f;
	}

	bool bHighPingAnimationValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingAnimation &&
		BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationValid)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration) {
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ABlasterPlayerController::ShowESCMenu()
{
	if (ESCMenuWidget == nullptr) return;

	if (ESCMenu == nullptr) {
		ESCMenu = CreateWidget<UESCWidget>(GetWorld(), ESCMenuWidget);
	}

	if (ESCMenu) {
		bESCMenuOpen = !bESCMenuOpen;
		if (bESCMenuOpen) {
			ESCMenu->MenuSetup();
		}
		else {
			ESCMenu->MenuTearDown();
		}
	}
}

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float GameOver, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	GameOverTime = GameOver;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (BlasterHUD && MatchState == MatchState::WaitingToStart) {
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode) {
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		GameOverTime = GameMode->GameOverTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, GameOverTime, LevelStartingTime);
	}

	if (LobbyGameMode) {
		WarmupTime = LobbyGameMode->WarmupTime;
		MatchTime = LobbyGameMode->MatchTime;
		GameOverTime = LobbyGameMode->GameOverTime;
		LevelStartingTime = LobbyGameMode->LevelStartingTime;
		MatchState = MatchState::InProgress;
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, GameOverTime, LevelStartingTime);
	}
}


void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) {
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress) {
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::GameOver) {
		TimeLeft = GameOverTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());

	//if (HasAuthority()) {
	//	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;

	//	if (BlasterGameMode) {
	//		SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetGameOverTime() + LevelStartingTime);
	//	}
	//}

	ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	if (LobbyGameMode == nullptr && CountdownInt != SecondsLeft) {
		if (MatchState == MatchState::WaitingToStart) {
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress) {
			SetHUDMatchCountdown(TimeLeft);
		}
		if (MatchState == MatchState::GameOver) {
			SetHUDAnnouncementCountdown(TimeLeft);          // change this to show scores after game
		}
	}

	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr) {
		if (BlasterHUD && BlasterHUD->CharacterOverlay) {
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay) {
				if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if(bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if(bInitializeScore) SetHUDScore(HUDScore);
				if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if(bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				if(bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
			}
		}
	}	
}


void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	}
	else {
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
	}
	else {
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->KillCount;

	if (bHUDValid) {
		FString KillText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Score));
		BlasterHUD->CharacterOverlay->KillCount->SetText(FText::FromString(KillText));
	}
	else {
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter) {
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
		SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
	}
}


void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DeathCount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DeathCount->SetText(FText::FromString(DefeatsText));
	}
	else {
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDBounty()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->TopPlayerName;

	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

	if (bHUDValid) {
		if (CurrentBountyPlayerState == BlasterPlayerState) {
			FString BountyPlayerName = "You are the Bounty";
			BlasterHUD->CharacterOverlay->TopPlayerName->SetText(FText::FromString(BountyPlayerName));
		}
		else if (CurrentBountyPlayerState) {
			FString BountyPlayerName = CurrentBountyPlayerState->GetPlayerName();
			BlasterHUD->CharacterOverlay->TopPlayerName->SetText(FText::FromString(BountyPlayerName));
		}
	}
}

void ABlasterPlayerController::SetHUDBountyPlayer()
{
	BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;
	if (HasAuthority() && BlasterGameState && BlasterGameState->bBountyUpdated) {
		ABlasterPlayerState* BountyPlayerState = BlasterGameState->TopScoringPlayer;

		if (BountyPlayerState != nullptr && BountyPlayerState != CurrentBountyPlayerState) {
			bBountyChanged = true;
			CurrentBountyPlayerState = BountyPlayerState;
		}
		else {
			bBountyChanged = false;
			BlasterGameState->bBountyUpdated = false;
			return;
		}
	}
	else if (HasAuthority()) {
		bBountyChanged = false;
		BlasterGameState->bBountyUpdated = false;
		return;
	}

	if (bBountyChanged) {
		SetHUDBounty();
	}
}

void ABlasterPlayerController::SetHUDWeaponImage(EWeaponType WeaponType, int32 Slot)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay;
	if (bHUDValid)
	{
		if (Slot == 3) {
			BlasterHUD->CharacterOverlay->SwapHUDWeaponImage();
		}
		else {
			BlasterHUD->CharacterOverlay->SetHUDWeaponImage(WeaponType, Slot);
		}
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoCount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoCount->SetText(FText::FromString(AmmoText));
	}
	else {
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoCount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoCount->SetText(FText::FromString(AmmoText));
	}
	else {
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(int32 CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid) {
		if (CountdownTime < 0.f) {
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60.f);

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);;
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f) {
			BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(FString("00:00")));
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}


void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress) {
		HandleMatchHasStarted();
	} 
	else if (MatchState == MatchState::GameOver) {
		HandleGameOver();
	}
}


void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress) {
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::GameOver) {
		HandleGameOver();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD) {
		if(BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->Announcement) {
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleGameOver()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD) {
		BlasterHUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = BlasterHUD->Announcement &&
			BlasterHUD->Announcement->AnnouncementText;

		if (bHUDValid) {
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);  // this is where you need to set winner screen or results
			FString AnnouncementText("New Match Starts In:");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
		}
	}

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter && BlasterCharacter->GetCombat()) {
		BlasterCharacter->bDisableGameplay = true;
		BlasterCharacter->GetCombat()->FireButtonPressed(false);
	}

}
