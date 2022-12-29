// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAICharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/Weapon/Weapon.h" 
#include "Blaster/BlasterComponents/CombatComponent.h" 
#include <Kismet/GameplayStatics.h>
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blaster/AI/BlasterAIController.h"
#include "Perception/AISense_Sight.h"
#include <Kismet/KismetMathLibrary.h>
#include "NavigationSystem.h"



ABlasterAICharacter::ABlasterAICharacter()
{
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
	PawnSensingComponent->SetIsReplicated(true);
	PawnSensingComponent->bSeePawns = false;
	PawnSensingComponent->bHearNoises = false;
	PawnSensingComponent->SightRadius = 3000.f;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	AIPerceptionComponent->SetIsReplicated(true);
}

void ABlasterAICharacter::BeginPlay()
{
	Super::BeginPlay();

	BlasterCombat = GetCombat();

	SpawnRandomWeapon();
	bIsInvinsible = false;

	if (HasAuthority() && PawnSensingComponent) {
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABlasterAICharacter::OnTargetUpdate);
	}
	AIController = Cast<ABlasterAIController>(GetController());

	GetWorldTimerManager().SetTimer(
		AIScanTargeTimer,
		this,
		&ABlasterAICharacter::SetCurrentTarget,
		AIScanTargetDelay,
		true
	);
}

void ABlasterAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsElimmed()) {
		if (TargetLostTimer.IsValid()) {
			GetWorldTimerManager().ClearTimer(TargetLostTimer);

		}
		if (ShootTimer.IsValid()) {
			GetWorldTimerManager().ClearTimer(ShootTimer);

		}
		if (StrafeTimer.IsValid()) {
			GetWorldTimerManager().ClearTimer(StrafeTimer);
		}
		StopAttacking();
	}
	
	if (bIsLockedOn) {
		if (!bIsMoving) {
			AILookAtPlayer(DeltaTime);
		}
		AttackTarget();
	}
	else {
		PatrolForTarget();
	}

	CheckValidTarget();
}

void ABlasterAICharacter::PatrolForTarget()
{
	if (AIController == nullptr) return;

	FNavLocation Location{};
	const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (NavSystem) {
		NavSystem->GetRandomReachablePointInRadius(GetActorLocation(), SearchRadius, Location);
	}

	if (!bIsLockedOn && CurrentTarget == nullptr && AIController->GetMoveStatus() == EPathFollowingStatus::Idle) {
		AIController->MoveTo(Location.Location);
		bIsPatrolling = true;
	}
}

void ABlasterAICharacter::SpawnRandomWeapon()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;
	int32 Selection = FMath::RandRange(0, SpawnWeaponList.Num() - 1);
	AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(SpawnWeaponList[Selection]);

	if (StartingWeapon) {
		BlasterCombat = GetCombat();
		if (BlasterCombat) {
			StartingWeapon->bDestroyWeapon = true;
			BlasterCombat->EquipWeapon(StartingWeapon);
		}
	}
}

ABlasterCharacter* ABlasterAICharacter::GetNearestTarget()
{
	if (AIPerceptionComponent == nullptr || !HasAuthority()) return nullptr;

	TArray<AActor*> SensedActors;
	AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SensedActors);
	float LeastDistance = (float)1e+4;
	ABlasterCharacter* NearestActor = nullptr;
	for (auto Target : SensedActors)
	{
		float Distance = GetDistanceTo(Target);
		if (Distance < LeastDistance) {
			ABlasterCharacter* TempTarget = Cast<ABlasterCharacter>(Target);
			if (TempTarget != nullptr && !TempTarget->IsElimmed() && TempTarget->IsPlayerControlled()) {
				NearestActor = TempTarget;
				LeastDistance = Distance;
			}
		}
	}

	//if (NearestActor != nullptr) {
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, NearestActor->GetName());
	//}
	//else {
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("None")));
	//}

	return NearestActor;
}

void ABlasterAICharacter::SetCurrentTarget()
{
	ABlasterCharacter* NearestActor = GetNearestTarget();
	if (NearestActor != nullptr && CurrentTarget != NearestActor) {
		CurrentTarget = NearestActor;
		AIController->GetBlackboardComponent()->SetValueAsObject("CurrentTarget", CurrentTarget);
		bIsLockedOn = true;
		bIsAttacking = false;
		bIsMoving = false;
	}
	else if (NearestActor == nullptr && !TargetLostTimer.IsValid()) {
		GetWorldTimerManager().SetTimer(
			TargetLostTimer,
			this,
			&ABlasterAICharacter::TargetLostTimerFinished,
			TargetLostDelay
		);
	}
}

void ABlasterAICharacter::AttackTarget()
{
	if (CurrentTarget == nullptr || AIController == nullptr) return;
	if (bIsPatrolling) {
		bIsPatrolling = false;
		AIController->StopMovement();
	}

	float DistanceToTarget = GetDistanceTo(CurrentTarget);
	if (bIsAttacking) {
		if (DistanceToTarget > AcceptableAttackRadius && AIController->GetMoveStatus() == EPathFollowingStatus::Idle) {
			bIsAttacking = false;
			bIsMoving = true;
			MoveToTarget = CurrentTarget;
			AIController->MoveToActor(CurrentTarget);
		}
	}
	else {
		if (DistanceToTarget <= AttackRadius) {
			if (AIController->GetMoveStatus() == EPathFollowingStatus::Moving) {
				AIController->StopMovement();
				bIsMoving = false;
			}
			StartAttacking();
		}
		else if (DistanceToTarget > AttackRadius && AIController->GetMoveStatus() == EPathFollowingStatus::Idle) {
			bIsMoving = true;
			bIsAttacking = false;
			MoveToTarget = CurrentTarget;
			AIController->MoveToActor(CurrentTarget);
		}
	}
}

bool ABlasterAICharacter::CheckValidTarget()
{
	if (CurrentTarget == nullptr) return false;
	if (CurrentTarget != nullptr && CurrentTarget->IsElimmed()) {
		StopAttacking();
		return false;
	}
	return true;
}

void ABlasterAICharacter::StartAttacking()
{
	if (bIsAttacking || AIController == nullptr || CurrentTarget == nullptr) return;
	bIsAttacking = true;

	StartShooting();

	GetWorldTimerManager().SetTimer(
		ShootTimer,
		this,
		&ABlasterAICharacter::ShootTimerFinished,
		ShootDelay,
		false
	);
}

void ABlasterAICharacter::StartShooting()
{
	if (BlasterCombat) {
		BlasterCombat->SetAiming(true);
		BlasterCombat->FireButtonPressed(true);
	}
}

void ABlasterAICharacter::StopShooting()
{
	if (BlasterCombat) {
		BlasterCombat->SetAiming(false);
		BlasterCombat->FireButtonPressed(false);
	}
}

void ABlasterAICharacter::StopAttacking()
{
	StopShooting();
	bIsAttacking = false;
	bIsLockedOn = false;
	CurrentTarget = nullptr;
	if (AIController == nullptr) return;
	if (bIsMoving) {
		AIController->StopMovement();
		bIsMoving = false;
	}
	AIController->GetBlackboardComponent()->SetValueAsObject("CurrentTarget", CurrentTarget);
	
}

void ABlasterAICharacter::AILookAtPlayer(float DeltaTime)
{
	if (CurrentTarget == nullptr || (CurrentTarget != nullptr && CurrentTarget->IsElimmed())) return;

	FVector PlayerLoc = CurrentTarget->GetActorLocation();
	FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerLoc);
	FRotator AIRot = FMath::RInterpTo(GetActorRotation(), PlayerRot, DeltaTime, AIRotationInterpSpeed);
	AIRot.Pitch = 0.f;
	AIRot.Roll = 0.f;
	SetActorRotation(AIRot);

	BlasterCombat->HitTarget = CurrentTarget->GetActorLocation();
}

void ABlasterAICharacter::OnTargetUpdate(AActor* Actor, FAIStimulus Stimulus)
{
	SensedPawn = Cast<ABlasterCharacter>(Actor);
	if (SensedPawn == nullptr) return;

	ABlasterCharacter* NearestActor = GetNearestTarget();

	if (Stimulus.WasSuccessfullySensed()) {
		if (TargetLostTimer.IsValid() && CurrentTarget == SensedPawn) {
			GetWorldTimerManager().ClearTimer(TargetLostTimer);
		}
	}
	else if (SensedPawn == CurrentTarget) {
		GetWorldTimerManager().SetTimer(
			TargetLostTimer,
			this,
			&ABlasterAICharacter::TargetLostTimerFinished,
			TargetLostDelay
		);
	}
}

void ABlasterAICharacter::TargetLostTimerFinished()
{
	ABlasterCharacter* NearestActor = GetNearestTarget();
	if (AIController) {
		if (NearestActor == nullptr) {
			StopAttacking();
			return;
		}
	}
}

void ABlasterAICharacter::StrafeTimerFinished()
{
	if (AIController == nullptr) return;
	AIController->StopMovement();

	if (!bIsAttacking) return;

	StartShooting();

	GetWorldTimerManager().SetTimer(
		ShootTimer,
		this,
		&ABlasterAICharacter::ShootTimerFinished,
		ShootDelay,
		false
	);
}

void ABlasterAICharacter::ShootTimerFinished()
{
	StopShooting();

	if (!bIsAttacking || AIController == nullptr) return;

	GetWorldTimerManager().SetTimer(
		StrafeTimer,
		this,
		&ABlasterAICharacter::StrafeTimerFinished,
		StrafeDelay,
		false
	);

	if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle) {
		bool bFoundLocation = false;

		while (!bFoundLocation)
		{
			FNavLocation Location{};
			const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

			if (NavSystem) {
				NavSystem->GetRandomReachablePointInRadius(GetActorLocation(), StrafeRadius, Location);
			}

			FRotator PlayerRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Location.Location);
			if (abs(PlayerRot.Yaw) < 90.f) {
				bFoundLocation = true;
				AIController->MoveTo(Location.Location);
			}
		}
	}
}
