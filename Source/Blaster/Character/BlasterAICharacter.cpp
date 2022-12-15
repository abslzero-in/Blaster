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



ABlasterAICharacter::ABlasterAICharacter()
{
	BlasterCombat = GetCombat();

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

	SpawnRandomWeapon();
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

	if (bIsAttacking) {
		AILookAtPlayer(DeltaTime);
		AttackTarget();
	}
}

void ABlasterAICharacter::SpawnRandomWeapon()
{	
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	int32 Selection = FMath::RandRange(0, SpawnWeaponList.Num() - 1);
	AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(SpawnWeaponList[Selection]);

	if (BlasterGameMode && World && StartingWeapon) {
		BlasterCombat = GetCombat();
		if (BlasterCombat) {
			StartingWeapon->bDestroyWeapon = true;
			BlasterCombat->EquipWeapon(StartingWeapon);
		}
	}
}

ABlasterCharacter* ABlasterAICharacter::GetNearestTarget()
{
	if (AIPerceptionComponent == nullptr) return nullptr;

	TArray<AActor*> SensedActors;
	AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SensedActors);
	float LeastDistance = (float) 1e+4;
	ABlasterCharacter* NearestActor = nullptr;
	for (auto Target : SensedActors)
	{
		float Distance = GetDistanceTo(Target);
		if (Distance < LeastDistance) {
			NearestActor = Cast<ABlasterCharacter>(Target);
			LeastDistance = Distance;
		}
	}

	if (NearestActor) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, NearestActor->GetName());
	}

	return NearestActor;
}

void ABlasterAICharacter::SetCurrentTarget()
{
	ABlasterCharacter* NearestActor = GetNearestTarget();
	if (NearestActor != nullptr) {
		CurrentTarget = NearestActor;
		AIController->GetBlackboardComponent()->SetValueAsObject("CurrentTarget", CurrentTarget);
	}
	else if (NearestActor == nullptr && !TargetLostTimer.IsValid()) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Some variab")));
		GetWorldTimerManager().SetTimer(
			TargetLostTimer,
			this,
			&ABlasterAICharacter::TargetLostTimerFinished,
			TargetLostDelay
		);
	}
}

void ABlasterAICharacter::StartAttacking()
{
	bIsAttacking = true;
	if (BlasterCombat) {
		BlasterCombat->FireButtonPressed(true);
	}
}

void ABlasterAICharacter::StopAttacking()
{
	bIsAttacking = false;
	if (BlasterCombat) {
		BlasterCombat->FireButtonPressed(false);
	}
}

void ABlasterAICharacter::AttackTarget()
{
	if (CurrentTarget == nullptr || (CurrentTarget != nullptr && CurrentTarget->IsElimmed())) {
		StopAttacking();
		return;
	}
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
		if (TargetLostTimer.IsValid() && CurrentTarget == NearestActor) {
			GetWorldTimerManager().ClearTimer(TargetLostTimer);
		}
		if (CurrentTarget != NearestActor) {
			CurrentTarget = NearestActor;
			if (AIController) {
				AIController->GetBlackboardComponent()->SetValueAsObject("CurrentsTarget", CurrentTarget);
				StartAttacking();
			}
		}
	}
	else if(SensedPawn == CurrentTarget) {
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
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("lost")));
		}
		CurrentTarget = NearestActor;
		AIController->GetBlackboardComponent()->SetValueAsObject("CurrentTarget", CurrentTarget);
	}
}