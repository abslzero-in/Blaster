// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


ABlasterAIController::ABlasterAIController()
{
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void ABlasterAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorTree == nullptr) {
		return;
	}

	RunBehaviorTree(BehaviorTree);
	BehaviorTreeComponent->StartTree(*BehaviorTree);
}

void ABlasterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (Blackboard != nullptr && BehaviorTree != nullptr) {
		Blackboard->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	}
}
