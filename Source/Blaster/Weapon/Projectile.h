#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"


#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	// server side rewind

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;

	float Damage = 20.f;

protected:
	virtual void BeginPlay() override;

	void SpawnTrailSystem();
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;


	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileInnerDamageRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileOuterDamageRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileMinDamage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileMaxDamage = 100.f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileDamageFalloffCurve = 1.f;

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 2.f;


};
