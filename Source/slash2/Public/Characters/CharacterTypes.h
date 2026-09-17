#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	// 未装备状态(将蓝图的状态变量名称ECS_Unequipped改为Unequipped)
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped OneHanded Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped TwoHanded Weapon")
};
UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
    EAS_HitReaction UMETA(DisplayName = "HitReaction"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_EquippingWeapon UMETA(DisplayName = "Eqipping Weapon"),
	EAS_Dead UMETA(DisplayName = "Dead")
};
UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	EDP_DeathFromBehind UMETA(DisplayName = "DeathFromBehind"),
	EDP_DeathFromFront UMETA(DisplayName = "DeathFromFront"),
	EDP_DeathFromLeft UMETA(DisplayName = "DeathFromLeft"),
	EDP_DeathFromRight UMETA(DisplayName = "DeathFromRight"),
	
	EDP_MAX UMETA(DisplayName = "DefaultMAX")
};
UENUM(BlueprintType)
enum class EEnemyState : uint8
{

	EES_NoState UMETA(DisplayName = "NoState"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
    EES_Engaged UMETA(DisplayName = "Engaged")

};