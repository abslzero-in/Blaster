// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Components/WidgetComponent.h>
#include <Components/CapsuleComponent.h>
#include <Components/BoxComponent.h>
#include <Net/UnrealNetwork.h>
#include "Blaster/Weapon/Weapon.h" 
#include "Blaster/BlasterComponents/CombatComponent.h" 
#include "Blaster/BlasterComponents/BuffComponent.h" 
#include "BlasterAnimInstance.h" 
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/AI/BlasterAIController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include <TimerManager.h>
#include <Sound/SoundCue.h>
#include <Particles/ParticleSystemComponent.h>
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include <NiagaraComponent.h>
#include <NiagaraFunctionLibrary.h>
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/GameMode/LobbyGameMode.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	LagCompensation->SetIsReplicated(true);
	

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 720.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	// hit boxes


	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes) {
		if (Box.Value) {
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);

	HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState && BlasterGameState->TopScoringPlayer == BlasterPlayerState) {
				MulticastGainedLead();
			}
		}
	}

	if (BlasterPlayerController == nullptr)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if (BlasterPlayerController)
		{
			SpawnDefaultWeapon();
			UpdateHUDAmmo();
			UpdateHUDHealth();
			UpdateHUDShield();
		}
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat) {
		Combat->Character = this;
	}
	if (Buff) {
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelolcity(GetCharacterMovement()->JumpZVelocity);
	}
	if(LagCompensation) {
		LagCompensation->Character = this;
		if (Controller) {
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) {
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}

	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (LobbyGameMode) {
		bIsInvinsible = true;
	}
	if (BlasterGameMode) {
		bIsInvinsible = false;
	}
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction(TEXT("SwapWeapons"), IE_Pressed, this, &ABlasterCharacter::SwapButtonPressed);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ABlasterCharacter::LookUp);
}

void ABlasterCharacter::MulticastGainedLead_Implementation()
{
	if (CrownSystem == nullptr) return;

	if (CrownComponent == nullptr) {
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 100.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}

	if (CrownComponent) {
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostLead_Implementation()
{
	if (CrownComponent) {
		CrownComponent->DestroyComponent();
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) {
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::StopReloadMontage(ECombatState CurrentCombatState)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage) {
		AnimInstance->Montage_Stop(0.f, ReloadMontage);
	}
	Combat->bLocallyReloading = false;
	Combat->CombatState = CurrentCombatState;
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage) {
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SMG:
			SectionName = FName("Pistol");	// Using pistol anim for smg
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage) {
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage) {
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage) {
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("Front");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && Combat && Combat->EquippedWeapon) {
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimmed || bIsInvinsible) return;

	float DamageToHealth = Damage;
	if (Shield > 0) {
		if (Shield >= Damage) {
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else {
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, MaxShield);
			Shield = 0.f;
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health == 0.f) {
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();

		if (BlasterGameMode) {
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			ABlasterAIController* AttackerAIController = Cast<ABlasterAIController>(InstigatorController);
			if (AttackerAIController) {
				BlasterGameMode->PlayerEliminatedByAI(this, BlasterPlayerController);
			}
			if (BlasterPlayerController) {
				BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
			}
			else {
				BlasterGameMode->EnemyAIEliminated(this, AttackerController);
			}
		}

		if (LobbyGameMode) {
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			if (BlasterPlayerController == nullptr) {
				LobbyGameMode->EnemyAIEliminated(this);
			}
		}
	}

}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapon();
	MulticastElim(bPlayerLeftGame);
}

void ABlasterCharacter::DropOrDestroyWeapon()
{
	if (Combat) {
		if (Combat->EquippedWeapon) {
			DropWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon) {
			DropWeapon(Combat->SecondaryWeapon);
		}

	}
}

void ABlasterCharacter::DropWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;

	if (Weapon->bDestroyWeapon) {
		Weapon->Destroy();
	}
	else {
		Weapon->Dropped();
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent) {
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress) {
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (BlasterPlayerController) {
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();

	// start dissolve effect

	if (DissolveMaterialInstance) {
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Sharpness"), 5.f);
	}
	StartDissolve();

	// disable movement

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	bDisableGameplay = true;
	if (Combat) {
		Combat->FireButtonPressed(false);
	}
	// disable collision

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// spawn elim bot

	if (ElimBotEffect) {
		FVector ElimBotSpawnPoint(GetActorLocation());
		ElimBotSpawnPoint.Z += 200.f;

		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}

	if (CrownComponent) {
		CrownComponent->DestroyComponent();
	}

	if (ElimBotSound) {
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}

	if (IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) {
		ShowSniperScopeWidget(false);
	}

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (LobbyGameMode) {
		LobbyGameMode->RequestRespawn(this);
	}
	if (BlasterGameMode && !bLeftGame) {
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled()) {
		OnLeftGame.Broadcast();
	}
	if (ElimBotComponent) {
		ElimBotComponent->DestroyComponent();
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterGameMode && BlasterPlayerState) {
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController) {
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController) {
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();

	if (BlasterGameMode && World && !bElimmed && DefaultWeapon) {
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeapon);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat) {
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f) {
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));

		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f) {
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat) {
		ServerEquipButtonPress();
		Combat->EquipWeaponSetHUD(OverlappingWeapon);
	}
}

void ABlasterCharacter::ServerEquipButtonPress_Implementation()
{
	if (Combat) {
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::ServerSwapButtonPress_Implementation()
{
	if (Combat) {
		Combat->SwapWeapons();
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Crouch();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat) {
		Combat->Reload();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat) {
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat) {
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) {
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);

		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning) {
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;

		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir || !IsPlayerControlled()) {
		bRotateRootBone = false;

		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalcAO_Pitch();
}

void ABlasterCharacter::CalcAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled()) {
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();

	if (Speed > 0.f) {
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotatorLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotatorLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold) {
		if (ProxyYaw > TurnThreshold) {
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold) {
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else {
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bDisableGameplay) return;
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat) {
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat) {
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::SwapButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->ShouldSwapWeapons() && Combat->CombatState != ECombatState::ECS_Swapping) {
		ServerSwapButtonPress();
		PlaySwapMontage();
		Combat->CombatState = ECombatState::ECS_Swapping;
		bFinishedSwapping = false;
	}
}


void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > TurnInPlaceAngle) {
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -TurnInPlaceAngle) {
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning) {
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < TurnInPlaceAimAngle) {
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}


void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon) {
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold || bSniperScopeActive) {
		HidePlayerMesh();
	}
	else {
		UnHidePlayerMesh();
	}
}

void ABlasterCharacter::UnHidePlayerMesh()
{
	GetMesh()->SetVisibility(true);
	if (Combat) {
		if (Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh()) {
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh()) {
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::HidePlayerMesh()
{
	GetMesh()->SetVisibility(false);
	if (Combat) {
		if (Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh()) {
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh()) {
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth) {
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield) {
		PlayHitReactMontage();
	}
}


void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance) {
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline) {
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{

	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled() && IsPlayerControlled()) {
		if (OverlappingWeapon) {
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if(Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}


void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay) {
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;

		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled()) {
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalcAO_Pitch();
	}
}


