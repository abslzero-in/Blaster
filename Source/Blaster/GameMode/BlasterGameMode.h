// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"


namespace MatchState {
	extern BLASTER_API const FName GameOver;
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController);
	virtual void PlayerEliminatedByAI(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController);
	virtual void EnemyAIEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);

	void SpawnAI();

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 15.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 720.f;

	UPROPERTY(EditDefaultsOnly)
	float GameOverTime = 7.f;

	float LevelStartingTime = 0.f;

private:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetGameOverTime() const { return GameOverTime; }

	UPROPERTY(Category = AI, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABlasterCharacter> AIType;

	UPROPERTY(Category = AI, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float SearchRadius = 6000.f;

	UPROPERTY(Category = AI, EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float AISpawnCount = 6.f;
};
