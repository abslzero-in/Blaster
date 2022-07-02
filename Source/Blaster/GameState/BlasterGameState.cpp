// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayer);
	DOREPLIFETIME(ABlasterGameState, bBountyUpdated);
}

void ABlasterGameState::UpdateTopScore(class ABlasterPlayerState* ScoringPlayer)
{
	if (ScoringPlayer->GetScore() > TopScore && TopScore == 0.f) {
		TopScoringPlayer = ScoringPlayer;
		TopScore = ScoringPlayer->GetScore();
		bBountyUpdated = true;
	}
	if (ScoringPlayer->GetScore() > TopScore && TopScoringPlayer != ScoringPlayer) {
		TopScoringPlayer = ScoringPlayer;
		TopScore = ScoringPlayer->GetScore();
		bBountyUpdated = true;
	}
}