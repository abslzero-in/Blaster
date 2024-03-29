#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"

#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter;
	friend class ABlasterAICharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void EquipWeaponSetHUD(AWeapon* WeaponToEquip);
	void SwapWeapons();
	void ReloadEmptyWeapon();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void UpdateCarriedAmmo();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void DropEquippedWeapon();
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();
	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	bool bLocallyReloading = false;
	bool bPrimaryEquipped = true;

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void Fire();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDealy);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDealy);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;
		
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;


	EWeaponType FirstSlotWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CrosshairShootingFactorValue;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CrosshairAimFactorSpread;
	float CrosshairBaseSpread;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CrosshairShootingInterpSpeed;


	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;
	
	float CurrentFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	// automatic fire

	FTimerHandle FireTimer;

	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;


	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 5;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 15;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 15;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 25;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 25;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 25;
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

public:	
	bool ShouldSwapWeapons();
		
};
