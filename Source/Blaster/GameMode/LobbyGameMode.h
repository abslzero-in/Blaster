// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 15.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 720.f;

	UPROPERTY(EditDefaultsOnly)
	float GameOverTime = 7.f;

	float LevelStartingTime = 0.f;

	virtual void EnemyAIEliminated(class ABlasterCharacter* ElimmedCharacter);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter);

private:
	float CountdownTime = 0.f;

	UPROPERTY(Category = AI, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABlasterCharacter> AIType;

	UPROPERTY(Category = AI, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float SearchRadius = 6000.f;
};
