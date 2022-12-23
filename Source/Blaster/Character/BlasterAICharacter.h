// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterCharacter.h"
#include "BlasterAICharacter.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterAICharacter : public ABlasterCharacter
{
	GENERATED_BODY()

public:
	ABlasterAICharacter();
	void BeginPlay();
	virtual void Tick(float DeltaTime) override;

protected:

	void PatrolForTarget();
	void SpawnRandomWeapon();
	ABlasterCharacter* GetNearestTarget();
	void SetCurrentTarget();
	void StartAttacking();
	void StartShooting();
	void StopShooting();
	void StopAttacking();
	void AttackTarget();
	bool CheckValidTarget();

	void AILookAtPlayer(float DeltaTime);

	UFUNCTION()
	void OnTargetUpdate(AActor* Actor, FAIStimulus Stimulus);

private:
	ABlasterCharacter* SensedPawn;
	class ABlasterAIController* AIController;
	ABlasterCharacter* CurrentTarget;

	UPROPERTY(Category = AI, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class AWeapon>> SpawnWeaponList; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* BlasterCombat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UPawnSensingComponent* PawnSensingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionComponent* AIPerceptionComponent;

	FTimerHandle TargetLostTimer;

	UPROPERTY(EditDefaultsOnly)
	float TargetLostDelay = 3.f;

	void TargetLostTimerFinished();

	FTimerHandle StrafeTimer;

	UPROPERTY(EditDefaultsOnly)
	float StrafeDelay = 3.f;

	void StrafeTimerFinished();

	FTimerHandle ShootTimer;

	UPROPERTY(EditDefaultsOnly)
	float ShootDelay = 3.f;

	void ShootTimerFinished();


	FTimerHandle AIScanTargeTimer;

	UPROPERTY(EditDefaultsOnly)
	float AIScanTargetDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly)
	float AIRotationInterpSpeed = 1.f;

	bool bIsLockedOn = false;
	bool bIsMoving = false;
	bool bCanAttack = false;
	ABlasterCharacter* MoveToTarget;
	bool bIsAttacking = false;
	bool bIsPatrolling = false;
	
	UPROPERTY(EditDefaultsOnly)
	float AttackRadius = 1000.f;

	UPROPERTY(EditDefaultsOnly)
	float AcceptableAttackRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (AllowPrivateAccess = true))
	float StrafeRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (AllowPrivateAccess = true))
	float SearchRadius = 3000.f;
public:
	FORCEINLINE ABlasterCharacter* GetCurrentTarget() const { return CurrentTarget; }

};
