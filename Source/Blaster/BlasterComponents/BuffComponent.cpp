#include "BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	float MaxHealth = Character->GetMaxHealth();

	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, MaxHealth));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0 || Character->GetHealth() >= MaxHealth) {
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bShielding || Character == nullptr || Character->IsElimmed()) return;

	const float ShieldThisFrame = ShieldingRate * DeltaTime;
	float MaxShield = Character->GetMaxShield();

	Character->SetShield(FMath::Clamp(Character->GetShield() + ShieldThisFrame, 0.f, MaxShield));
	Character->UpdateHUDShield();
	AmountToShield -= ShieldThisFrame;

	if (AmountToShield <= 0 || Character->GetShield() >= MaxShield) {
		bShielding = false;
		AmountToShield = 0.f;
	}
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::Shield(float ShieldAmount, float ReplenishShieldTime)
{
	bShielding = true;
	ShieldingRate = ShieldAmount / ReplenishShieldTime;
	AmountToShield += ShieldAmount;
}

void UBuffComponent::BuffSpeed(float SpeedMultiplier, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);

	float BuffBaseSpeed = SpeedMultiplier * InitialBaseSpeed;
	float BuffCrouchSpeed = SpeedMultiplier * InitialCrouchSpeed;

	if (Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
		MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
	}
}

void UBuffComponent::ResetSpeeds()
{
	if (Character == nullptr) return;

	if (Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
		MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
	}
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (Character == nullptr) return;

	if (Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}


void UBuffComponent::BuffJump(float JumpZMultiplier, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	float BuffJumpZVelocity = JumpZMultiplier * InitialJumpVelocity;

	if (Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpZVelocity;
		MulticastJumpBuff(BuffJumpZVelocity);
	}
}

void UBuffComponent::ResetJump()
{
	if (Character == nullptr) return;

	if (Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
		MulticastJumpBuff(InitialJumpVelocity);
	}
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character == nullptr) return;

	if (Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::SetInitialJumpVelolcity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}




