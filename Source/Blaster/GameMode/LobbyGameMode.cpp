// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include <GameFramework/GameStateBase.h>
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "NavigationSystem.h"
#include "Blaster/Character/BlasterAICharacter.h"



void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumOfPLayers = GameState.Get()->PlayerArray.Num();

	if (NumOfPLayers == 2) {
		UWorld* World = GetWorld();
		if (World) {
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/BlasterAsianMap?listen"));
		}
	}
}

void ALobbyGameMode::EnemyAIEliminated(ABlasterCharacter* ElimmedCharacter)
{
	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(false);
	}
}


void ALobbyGameMode::RequestRespawn(ACharacter* ElimmedCharacter)
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
	}
}