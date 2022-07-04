// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();
	virtual void Destroyed() override;

protected:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	void DestroyTimerFinished();

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:

	UPROPERTY(EditAnywhere, Category="Rocket")
	float RocketInnerDamageRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Rocket")
	float RocketOuterDamageRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Rocket")
	float RocketMinDamage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Rocket")
	float RocketMaxDamage = 100.f;

	UPROPERTY(EditAnywhere, Category = "Rocket")
	float RocketDamageFalloffCurve = 1.f;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 2.f;
};
