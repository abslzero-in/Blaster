#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealingTime);
	void Shield(float ShieldAmount, float ReplenishShieldTime);

	void BuffSpeed(float SpeedMultiplier, float BuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

	void BuffJump(float JumpZMultiplier, float BuffTime);
	void SetInitialJumpVelolcity(float Velocity);

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	// heal

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	// shield

	bool bShielding = false;
	float ShieldingRate = 0.f;
	float AmountToShield = 0.f;

	// speed

	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();

	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	// jump

	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

};
