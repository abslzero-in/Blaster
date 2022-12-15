// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterBTTask_FindRandomLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBlasterBTTask_FindRandomLocation::UBlasterBTTask_FindRandomLocation()
{
	NodeName = TEXT("Find Random Location");

	// accept only vectors
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBlasterBTTask_FindRandomLocation, BlackboardKey));
}

EBTNodeResult::Type UBlasterBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* AIPawn = AIController->GetPawn();
	const FVector Origin = AIPawn->GetActorLocation();
	FNavLocation Location = GetRandomLocation(Origin, SearchRadius);

	AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, Location.Location);

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

FNavLocation UBlasterBTTask_FindRandomLocation::GetRandomLocation(FVector Origin, float Radius)
{
	FNavLocation Location{};
	const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (NavSystem) {
		NavSystem->GetRandomReachablePointInRadius(Origin, Radius, Location);
	}

	return Location;
}

FString UBlasterBTTask_FindRandomLocation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Vector: %s"), *BlackboardKey.SelectedKeyName.ToString());
}