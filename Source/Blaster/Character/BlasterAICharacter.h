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

	void SpawnRandomWeapon();
	ABlasterCharacter* GetNearestTarget();
	void SetCurrentTarget();
	void StartAttacking();
	void StopAttacking();
	void AttackTarget();

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


	FTimerHandle AIScanTargeTimer;

	UPROPERTY(EditDefaultsOnly)
	float AIScanTargetDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly)
	float AIRotationInterpSpeed = 1.f;

	bool bIsAttacking = false;
	bool bIsFiring = false;

public:
	FORCEINLINE ABlasterCharacter* GetCurrentTarget() const { return CurrentTarget; }

};
