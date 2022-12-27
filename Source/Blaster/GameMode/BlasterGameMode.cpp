// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Character/BlasterAICharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/PlayerStart.h>
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "AIController.h"
#include "NavigationSystem.h"


namespace MatchState {
	const FName GameOver = FName("GameOver");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}


void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
	SpawnAI();
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart) {
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f) {
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress) {
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f) {
			SetMatchState(MatchState::GameOver);
		}
	}
	else if (MatchState == MatchState::GameOver) {
		CountdownTime = GameOverTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f) {
			RestartGame();
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator i = GetWorld()->GetPlayerControllerIterator(); i; i++) {
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*i);
		if (BlasterPlayer) {
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		ABlasterPlayerState* PlayerCurrentlyInLead = BlasterGameState->TopScoringPlayer;

		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		if (BlasterGameState->TopScoringPlayer == AttackerPlayerState) {
			ABlasterCharacter* TopPlayer = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if (TopPlayer) {
				TopPlayer->MulticastGainedLead();
			}
		}

		if (PlayerCurrentlyInLead && BlasterGameState->TopScoringPlayer != PlayerCurrentlyInLead) {
			ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayerCurrentlyInLead->GetPawn());
			if (Loser) {
				Loser->MulticastLostLead();
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			BlasterPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ABlasterGameMode::PlayerEliminatedByAI(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController)
{
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(false);
	}
}

void ABlasterGameMode::EnemyAIEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;

	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && BlasterGameState)
	{
		ABlasterPlayerState* PlayerCurrentlyInLead = BlasterGameState->TopScoringPlayer;

		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		if (BlasterGameState->TopScoringPlayer == AttackerPlayerState) {
			ABlasterCharacter* TopPlayer = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if (TopPlayer) {
				TopPlayer->MulticastGainedLead();
			}
		}

		if (PlayerCurrentlyInLead && BlasterGameState->TopScoringPlayer != PlayerCurrentlyInLead) {
			ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayerCurrentlyInLead->GetPawn());
			if (Loser) {
				Loser->MulticastLostLead();
			}
		}
	}

	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(false);
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter) {
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();

		ABlasterAICharacter* ElimmedAICharacter = Cast<ABlasterAICharacter>(ElimmedCharacter);

		if (ElimmedAICharacter) {

			FVector LastLocation = ElimmedCharacter->GetActorLocation();
			FRotator LastRotation = ElimmedCharacter->GetActorRotation();
			FActorSpawnParameters SpawnParams;
			UWorld* World = GetWorld();
			FNavLocation Location{};
			const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

			ElimmedCharacter->Reset();
			ElimmedCharacter->Destroy();

			if (NavSystem) {
				NavSystem->GetRandomReachablePointInRadius(LastLocation, SearchRadius, Location);
			}

			Location.Location.Z += 300.f;	// to make sure it doesnt spawn inside terrain

			ABlasterAICharacter* SpawnedAI = World->SpawnActor<ABlasterAICharacter>(AIType, Location.Location, LastRotation);
		}
		else {
			if (ElimmedController) {
				TArray<AActor*> PlayerStarts;
				UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
				int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
				RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
			}
		}
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (BlasterGameState && BlasterGameState->TopScoringPlayer == PlayerLeaving) {
		BlasterGameState->TopScoringPlayer = nullptr;					// TODO: track score of all players
	}
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving) {
		CharacterLeaving->Elim(true);
	}
}

void ABlasterGameMode::SpawnAI()
{
	TArray<AActor*> PlayerStarts;
	UWorld* World = GetWorld();
	UGameplayStatics::GetAllActorsOfClassWithTag(this, AActor::StaticClass(), FName("AISpawnPoint"), PlayerStarts);
	if (PlayerStarts.Num() <= 0) return;
	for (int32 i = 0; i < AISpawnCount; i++)
	{
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		FVector LastLocation = PlayerStarts[Selection]->GetActorLocation();
		FRotator LastRotation = PlayerStarts[Selection]->GetActorRotation();
		ABlasterAICharacter* SpawnedAI = World->SpawnActor<ABlasterAICharacter>(AIType, LastLocation, LastRotation);
	}
}


