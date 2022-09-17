#pragma once


UENUM(BlueprintType)
enum class ECombatState : uint8 {
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Swapping UMETA(DisplayName = "Swapping"),


	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};