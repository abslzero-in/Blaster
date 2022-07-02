// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayer);
}

bool ABlasterGameState::UpdateTopScore(class ABlasterPlayerState* ScoringPlayer)
{
	if (ScoringPlayer->GetScore() > TopScore) {                // Add  && TopScoringPlayer != ScoringPlayer
		TopScoringPlayer = ScoringPlayer;
		TopScore = ScoringPlayer->GetScore();
		return true;
	}

	return false;
}