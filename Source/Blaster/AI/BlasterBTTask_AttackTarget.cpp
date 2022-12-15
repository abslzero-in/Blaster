// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterBTTask_AttackTarget.h"
#include "GameFramework/Actor.h"
#include "AIController.h"
#include "Blaster/Character/BlasterAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"


UBlasterBTTask_AttackTarget::UBlasterBTTask_AttackTarget()
{
	NodeName = TEXT("Attack Target");

	// accept only actors
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBlasterBTTask_AttackTarget, BlackboardKey), ABlasterCharacter::StaticClass());
}

EBTNodeResult::Type UBlasterBTTask_AttackTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AIPawn = AIPawn == nullptr ? Cast<ABlasterAICharacter>(AIController->GetPawn()) : AIPawn;

	ABlasterCharacter* AttackTarget = AIPawn->GetCurrentTarget();

	if (AIController != nullptr && AttackTarget != nullptr && AttackTarget != CurrentTarget) {
		CurrentTarget = AttackTarget;
		AIController->GetBlackboardComponent()->SetValueAsObject(BlackboardKey.SelectedKeyName, CurrentTarget);
	}
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}

FString UBlasterBTTask_AttackTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Actor: %s"), *BlackboardKey.SelectedKeyName.ToString());
}
